/*
	PhysicsSystem.cpp
	20250821  hanaue sho
	�������Z
*/
#include <assert.h>
#include <algorithm>
#include "PhysicsSystem.h"
#include "Scene.h"
#include "Component.h"
#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "RigidbodyComponent.h"
#include "Collision.h"
#include "ContactManifold.h"
namespace
{
	Vector3 Gravity = { 0.0f, -9.8f, 0.0f }; // �K�v�Ȃ�O
	const float Slop = 0.005f;				 // ���e�ʁi�傫������Ƃ߂荞�ނ�j
	const float RestThreshold = 0.2f;		 // �����o�C�A�X���e��
	const float Baumgarte = 0.2f;			 // �ʒu�␳�̌W��

	// �N�����v
	float Clamp(float value, float min, float max)
	{
		return value < min ? min : (value > max ? max : value);
	}
}


void PhysicsSystem::Init()
{
	m_Colliders.clear();
	m_PrevTrigger.clear();
	m_CurrTrigger.clear();
	m_PrevCollision.clear();
	m_CurrCollision.clear();
	m_TriggerEnter.clear();
	m_TriggerExit.clear();
	m_CollisionEnter.clear();
	m_CollisionExit.clear();
	m_NextId = 0; m_FreeIds.clear(); m_ById.clear();
}

void PhysicsSystem::Shutdown()
{
	m_Colliders.clear();
	m_PrevTrigger.clear();
	m_CurrTrigger.clear();
	m_PrevCollision.clear();
	m_CurrCollision.clear();
	m_TriggerEnter.clear();
	m_TriggerExit.clear();
	m_CollisionEnter.clear();
	m_CollisionExit.clear();
	m_pScene = nullptr;
}

int PhysicsSystem::RegisterCollider(Collider* c)
{
	assert(c);
	assert(c->m_Id == -1 && "already registered");

	int id;
	if (!m_FreeIds.empty()) { id = m_FreeIds.back(); m_FreeIds.pop_back(); }
	else { id = m_NextId++; m_ById.resize(m_NextId, nullptr); }
	c->m_Id = id;
	m_ById[id] = c; // �t�����p�ɓo�^

	c->UpdateWorldAABB();
	m_Colliders.push_back(c);
	return id;
}

void PhysicsSystem::UnregisterCollider(Collider* c)
{
	if (!c || c->m_Id < 0) return;

	// �y�A�W�����珜��
	const int id = c->m_Id;
	for (auto it = m_PrevTrigger.begin(); it != m_PrevTrigger.end();)
	{
		int a = KeyHigh(*it), b = KeyLow(*it);
		if (a == id || b == id) it = m_PrevTrigger.erase(it);
		else it++;
	}
	for (auto it = m_CurrTrigger.begin(); it != m_CurrTrigger.end();)
	{
		int a = KeyHigh(*it), b = KeyLow(*it);
		if (a == id || b == id) it = m_CurrTrigger.erase(it);
		else it++;
	}
	for (auto it = m_PrevCollision.begin(); it != m_PrevCollision.end();)
	{
		int a = KeyHigh(*it), b = KeyLow(*it);
		if (a == id || b == id) it = m_PrevCollision.erase(it);
		else it++;
	}
	for (auto it = m_CurrCollision.begin(); it != m_CurrCollision.end();)
	{
		int a = KeyHigh(*it), b = KeyLow(*it);
		if (a == id || b == id) it = m_CurrCollision.erase(it);
		else it++;
	}

	// �t����
	m_ById[id] = nullptr;
	m_FreeIds.push_back(id);

	// ���X�g���珜�O
	auto it = std::find(m_Colliders.begin(), m_Colliders.end(), c);
	if (it != m_Colliders.end()) { *it = m_Colliders.back(); m_Colliders.pop_back(); }
	c->m_Id = -1;
}

// �X�e�b�v�̎n�܂�
// �X�N���v�g�� FixedUpdate ���Ɏ��s
void PhysicsSystem::BeginStep(float fixedDt)
{
	m_CurrTrigger.clear();
	m_CurrCollision.clear();
	m_Contacts.clear();

	// ----- �O�́����x�i�d�͂Ȃǁj-----
	IntegrationForce(fixedDt);

	// �X�e�b�v�̍ŏ��� WorldAABB ���X�V
	for (Collider* c : m_Colliders)
		c->UpdateWorldAABB();
}

void PhysicsSystem::Step(float fixedDt)
{
	// ----- �Փ˔��� -----
	DetermineCollision();

	// ----- ���x���� -----
	for (int i = 0; i < 8; i++) { ResolveVelocity(fixedDt); /*printf("---------- resolve : %d\n", i);*/ }
}

void PhysicsSystem::EndStep(float fixedDt)
{
	// ----- ���� -----
	ApplyDamping(fixedDt);

	// ----- ���x�ϕ� -----
	IntegrationVelocity(fixedDt);

	// ----- �ʒu�␳ -----
	for (int i = 0; i < 4; i++)	CorrectPosition();


	auto diffSets = [&](auto& prev, auto& curr, auto& outEnter, auto& outExit)
		{
			// Enter : Curr �ɂ����� Prev �ɂȂ�
			for (auto k: curr) 
				if (!prev.count(k))
				{
					outEnter.emplace_back(m_ById[KeyHigh(k)], m_ById[KeyLow(k)]); // Enter �ɂ����
				}
			// Exit : Prev �ɂ����� Curr �ɂȂ�
			for (auto k : prev) 
				if (!curr.count(k))
				{
					outExit.emplace_back(m_ById[KeyHigh(k)], m_ById[KeyLow(k)]); // Exit �ɂ����
				}
			prev.swap(curr); curr.clear();
		};

	diffSets(m_PrevTrigger  , m_CurrTrigger  , m_TriggerEnter  , m_TriggerExit);
	diffSets(m_PrevCollision, m_CurrCollision, m_CollisionEnter, m_CollisionExit);

	// ----- �f�B�X�p�b�`���� -----
	DispatchEvents(); 
}

void PhysicsSystem::DispatchEvents()
{
	// OnTriggerEnte, Exit �̌Ăяo��
	auto callAll = [](Collider* me, Collider* other, bool enter, bool isTrigger)
		{
			if (auto* go = me->Owner())
			{
				go->ForEachComponent(
					[&](Component* c) {
						if (isTrigger)
						{
							if (enter) c->OnTriggerEnter(me, other);
							else	   c->OnTriggerExit(me, other);
						}
						else
						{
							if (enter) c->OnCollisionEnter(me, other);
							else	   c->OnCollisionExit(me, other);
						}
					}); // �h�����_�h��n���Ă���
			}
		};

	// TriggerEnter -----
	for (auto& p : m_TriggerEnter)
	{
		Collider* A = p.first;
		Collider* B = p.second;
		callAll(A, B, true, true);
		callAll(B, A, true, true);
	}
	// TriggerExit -----
	for (auto& p : m_TriggerExit)
	{
		Collider* A = p.first;
		Collider* B = p.second;
		callAll(A, B, false, true);
		callAll(B, A, false, true);
	}
	// CollisionEnter -----
	for (auto& p : m_CollisionEnter)
	{
		Collider* A = p.first;
		Collider* B = p.second;
		callAll(A, B, true, false);
		callAll(B, A, true, false);
	}
	// CollisionExit -----
	for (auto& p : m_CollisionExit)
	{
		Collider* A = p.first;
		Collider* B = p.second;
		callAll(A, B, false, false);
		callAll(B, A, false, false);
	}

	// ��n��
	m_TriggerEnter.clear();
	m_TriggerExit.clear();
	m_CollisionEnter.clear();
	m_CollisionExit.clear();
}

// �͔��f
void PhysicsSystem::IntegrationForce(float dt)
{
	for (Collider* c : m_Colliders)
	{
		auto* rb = c->Owner()->GetComponent<Rigidbody>();
		if (!rb || !rb->IsDynamic()) continue;

		// ������ InertiaWorldInv �̍X�V -----
		rb->UpdateInertiaWorldInvFrom(c->Owner()->GetComponent<TransformComponent>()->WorldRotation());

		// �d�͉����x�̌���
		Vector3 gravity{ 0.0f, 0.0f, 0.0f };
		switch (rb->GravityMode())
		{
		case Rigidbody::GravityMode::Global: // �O���[�o���d��
			gravity = Gravity * rb->GravityScale();
			break;
		case Rigidbody::GravityMode::Custom: // �J�X�^���d��
			gravity = rb->CustomGravity();
			break;
		case Rigidbody::GravityMode::None: // ���d��
			gravity = {0.0f, 0.0f, 0.0f};
			break;
		}

		// a = g + F/m
		Vector3 force = rb->ConsumeForces();
		Vector3 acc = gravity + force * rb->InvMass();
		rb->SetVelocity(rb->Velocity() + acc * dt);

		// �g���N���f
		// �� += InertiaWorld^-1 * t * dt
		Vector3 tau = rb->ConsumeTorques();
		Vector3 omega = rb->ApplyInvInertiaWorld(tau) * dt;
		rb->SetAngularVelocity(rb->AngularVelocity() + omega);
	}
}

// �Փ˔���
void PhysicsSystem::DetermineCollision()
{
	// �����蔻��
	const int n = (int)m_Colliders.size();
	for (int ia = 0; ia < n; ia++)
	{
		// Collider �擾
		Collider* A = m_Colliders[ia];
		const Collision* shapeA = A->Shape();
		if (!shapeA) continue;

		const Transform& transA = A->Owner()->GetComponent<TransformComponent>()->Value();

		for (int ib = ia + 1; ib < n; ib++)
		{
			Collider* B = m_Colliders[ib];

			// �u���[�h�iAABB�j
			if (!A->WorldAABB().isOverlap(B->WorldAABB())) continue; // AABB �œ������Ă��邩��܂��ɔ���

			// �i���[
			const Collision* shapeB = B->Shape();
			if (!shapeB) continue;

			const Transform& transB = B->Owner()->GetComponent<TransformComponent>()->Value();

			ContactManifold m; 
			if (!shapeA->isOverlap(transA, *shapeB, transB, m, 0.0f)) continue;

			if (A->IsTrigger() || B->IsTrigger()) m_CurrTrigger.insert(MakePairKey(A->Id(), B->Id()));
			else // �ǂ����� IsTrigger �� off �������� Collision �C�x���g��			  
			{
				m_CurrCollision.insert(MakePairKey(A->Id(), B->Id()));
				if (m.touching && m.count > 0)
					m_Contacts.push_back({ A, B, m });
			}
		}
	}
}

// ���x����
// �����C���p���X�@�i�ڐG�A���C�A�W���C���g�Ȃǂ̑��x�S���ő����ɑ��x���X�V�����@�j
void PhysicsSystem::ResolveVelocity(float dt)
{
	for (auto& c : m_Contacts)
	{
		auto* rbA = c.A->Owner()->GetComponent<Rigidbody>();
		auto* rbB = c.B->Owner()->GetComponent<Rigidbody>();

		const float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f; // �t���ʎ擾
		const float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f; // �t���ʎ擾
		if (invA + invB == 0.0f) continue; // �����Œ�

		const Vector3 n = c.m.normal; // A��B

		// �����W��
		const float eCandidate = std::max(rbA ? rbA->Restitution() : 0.0f,
										  rbB ? rbB->Restitution() : 0.0f);

		// ���C�W���i���݂�Rigidbody�j
		const float fricDynamicA = rbA ? rbA->FrictionDynamic() : 0.0f;
		const float fricDynamicB = rbB ? rbB->FrictionDynamic() : 0.0f;
		const float fricStaticA = rbA ? rbA->FrictionStatic() : 0.0f;
		const float fricStaticB = rbB ? rbB->FrictionStatic() : 0.0f;
		auto Mix = [](float a, float b) {return (a > 0.0f && b > 0.0f) ? sqrtf(a * b) : std::max(a, b); };
		float fricD = Mix(fricDynamicA, fricDynamicB);
		float fricS = Mix(fricStaticA, fricStaticB);
		fricD = std::max(0.0f, fricD);
		fricS = std::max(0.0f, std::max(fricS, fricD)); // fricS >= fricD

		// ----- �p���̏��w���p -----
		// d : �]�������i�@���܂��͐ڐ��j
		// �ڐG�_�ɕ������̃C���p���X�� 1[N�Es]���ꂽ�Ƃ��́u�p�^���ɂ��t�����x��^�v���X�J���[�ŕԂ�
		// �ڐG�_���x(v = �ց~r)�ɃC���p���X(P = d * j)����ꂽ�Ƃ��̊p���x�ω�(���� = I^-1(r�~P))��ڐG�_���x�֖߂�����^((I^-1(r�~d))�~r)�����Ŏˉe�������́i�H�j
		auto AngTerm = [](Rigidbody* rb, const Vector3& r, const Vector3& d) -> float
			{
				// n�E[(I^-1 (r�~d))�~r] = dot(d, (r�~d))�~r)��W�J���Ă��������ǐ��d���ł�����
				return  Vector3::Dot(d, Vector3::CrossLH(rb->ApplyInvInertiaWorld(Vector3::CrossLH(r, d)), r));
			};

		
		// ======== PHASE 1: �S�ړ_�́u�@���v�������� =========
		// �e�ڐG�_�ɂ��Ė@���C���p���X
		Vector3 sumTor = Vector3();
		for (int i = 0; i < c.m.count; i++)
		{
			// �ڐG�_�� COM ���������
			const Vector3 p = (c.m.points[i].pointOnA + c.m.points[i].pointOnB) * 0.5f; //�ڐG�_�̒��_�����߂�
			const Vector3 pA = c.m.points[i].pointOnA;
			const Vector3 pB = c.m.points[i].pointOnB;
			const Vector3 xA = rbA ? rbA->WorldCOM() : Vector3(); // �d�S���W
			const Vector3 xB = rbB ? rbB->WorldCOM() : Vector3(); // �d�S���W
			const Vector3 rA = p - xA; // COM ����̃x�N�g���i���Έʒu�j
			const Vector3 rB = p - xB; // COM ����̃x�N�g���i���Έʒu�j

			// ���x�i�p�������݁j
			const Vector3 vA = rbA ? rbA->Velocity()		: Vector3{};
			const Vector3 wA = rbA ? rbA->AngularVelocity() : Vector3{};
			const Vector3 vB = rbB ? rbB->Velocity()		: Vector3{};
			const Vector3 wB = rbB ? rbB->AngularVelocity() : Vector3{};

			// �ڐG�_�̑��Α��x vRel = (vB + wB�~rB) - (vA + wA�~rA) [relative velocity]
			const Vector3 vRel = (vB + Vector3::CrossLH(wB, rB)) - (vA + Vector3::CrossLH(wA, rA)); // ���Α��x�iA���猩��B�̑��Α��x�j
			const float relVelN = Vector3::Dot(vRel, n); // �@�������̑��Α��x [normal relative velocity]
			if (relVelN > 0.0f && c.m.points[i].penetration <= Slop) continue; // ����Ă������� && �߂荞�݂����� �Ȃ̂ŕs�v


			// ----- �@���C���p���X�i�p�����݂̗L�����ʁj -----
			// �L�����ʂ̕���i�@���j
			float denomN = invA + invB; // �p�����݂̖@�������̗L�����ʂ̋t [normal effective mass]
			if (rbA && invA > 0.0f) denomN += AngTerm(rbA, rA, n); // �ڐG�_���x�́g�����������h���ǂꂾ���ς�邩��\���W����
			if (rbB && invB > 0.0f) denomN += AngTerm(rbB, rB, n); // ���� k = (invA + invB + AngTermA + AngTermB)
			if (denomN < 1e-12f) continue;

			// �ʒu�o�C�A�X
			const float pen = c.m.points[i].penetration;
			const float bias_pos = pen > Slop ? Baumgarte * (pen - Slop) / dt : 0.0f;
			// �����o�C�A�X
			const float bias_rest = relVelN < -RestThreshold ? (-eCandidate * relVelN) : 0.0f; // �����Ŕ����������܂܂��

			// �����C���p���X�i�C���p���X�ʁj
			float deltaImpulseN = -(relVelN + bias_pos + bias_rest) / denomN;

			// �ݐσN�����v
			float old = c.m.points[i].accumN;
			c.m.points[i].accumN = std::max(0.0f, old + deltaImpulseN);
			deltaImpulseN = c.m.points[i].accumN - old; // ���ۂɓK�p���鑝��

			const Vector3 impulseVector = n * deltaImpulseN; // �x�N�g���������C���p���X [normal impulse vector]

			// v �� �� �ɑ����K�p -----
			if (rbA && invA > 0.0f)
			{
				rbA->SetVelocity(vA - impulseVector * invA);
				rbA->SetAngularVelocity(wA - rbA->ApplyInvInertiaWorld(Vector3::CrossLH(rA, impulseVector)));
				sumTor += -rbA->ApplyInvInertiaWorld(Vector3::CrossLH(rA, impulseVector));
			}
			if (rbB && invB > 0.0f)
			{
				rbB->SetVelocity(vB + impulseVector * invB);
				rbB->SetAngularVelocity(wB + rbB->ApplyInvInertiaWorld(Vector3::CrossLH(rB, impulseVector)));
			}
			float signPribe = (p.x - xA.x) * deltaImpulseN;
			Vector3 dw = rbA->ApplyInvInertiaWorld(Vector3::CrossLH(p - xA, -n * deltaImpulseN));
			int f = 0;
		}
		
		// ======== PHASE 2: �S�ړ_�́u���C�v�������� =========
		// �e�ڐG�_�ɂ��Ė��C
		for (int i = 0; i < c.m.count; i++)
		{
			// �ڐG�_�� COM ���������
			const Vector3 p = (c.m.points[i].pointOnA + c.m.points[i].pointOnB) * 0.5f; //�ڐG�_�̒��_�����߂�
			const Vector3 pA = c.m.points[i].pointOnA;
			const Vector3 pB = c.m.points[i].pointOnB;
			const Vector3 xA = rbA ? rbA->WorldCOM() : Vector3(); // �d�S���W
			const Vector3 xB = rbB ? rbB->WorldCOM() : Vector3(); // �d�S���W
			const Vector3 rA = p - xA; // COM ����̃x�N�g���i���Έʒu�j
			const Vector3 rB = p - xB; // COM ����̃x�N�g���i���Έʒu�j

			// ----- �����C�́i�@���X�V��̑��Α��x�ŕ]���j -----
			const Vector3 vA2 = rbA ? rbA->Velocity() : Vector3{};
			const Vector3 wA2 = rbA ? rbA->AngularVelocity() : Vector3{};
			const Vector3 vB2 = rbB ? rbB->Velocity() : Vector3{};
			const Vector3 wB2 = rbB ? rbB->AngularVelocity() : Vector3{};
			const Vector3 vRel2 = (vB2 + Vector3::CrossLH(wB2, rB)) - (vA2 + Vector3::CrossLH(wA2, rA)); // ���Α��x

			// ----- ��]����ڐ��̃C���p���X -----
			Vector3 t1;
			if (fabsf(n.x) > 0.57735f) t1 = Vector3{ -n.y, n.x, 0.0f };
			else					   t1 = Vector3{ 0.0f, -n.z, n.y };
			t1 = t1.normalized();
			Vector3 t2 = Vector3::CrossLH(n, t1); // �ڐ������i�Q���ځj

			// �L�����ʂ̕���
			float denomT1 = invA + invB; // �ڐ������̗L�����ʂ̕��� [tangential impulse mass]
			if (rbA && invA > 0.0f) denomT1 += AngTerm(rbA, rA, t1);
			if (rbB && invB > 0.0f) denomT1 += AngTerm(rbB, rB, t1);
			if (denomT1 < 1e-12f) continue;
			float denomT2 = invA + invB; // �ڐ������̗L�����ʂ̕��� [tangential impulse mass]
			if (rbA && invA > 0.0f) denomT2 += AngTerm(rbA, rA, t2);
			if (rbB && invB > 0.0f) denomT2 += AngTerm(rbB, rB, t2);
			if (denomT2 < 1e-12f) continue;

			// �]�܂��������i�Î~���C�^�[�Q�b�g vt' = 0�j
			float v_t1 = Vector3::Dot(vRel2, t1);
			float v_t2 = Vector3::Dot(vRel2, t2);
			float deltaImpulseT_desired_1 = -(v_t1) / denomT1; // [tangential impulse scalar]
			float deltaImpulseT_desired_2 = -(v_t2) / denomT2; // [tangential impulse scalar]

			// ----- Coulomb �~���i���a = �� * jn�j�ŃN�����v -----
			float jtMaxS = fricS * c.m.points[i].accumN; // �Î~���C���a�@�� * jn �i�ݐρj
			float jtMaxD = fricD * c.m.points[i].accumN; // �����C���a�@�� * jn �i�ݐρj
			if (jtMaxD <= 0.0f) continue; // �@�����͂��Ȃ������C������

			Vector3 oldImpWorld = c.m.points[i].accumImpulseT; // ���[���h�ݐ�
			float oldImp1 = Vector3::Dot(oldImpWorld, t1); // t1 �������o��
			float oldImp2 = Vector3::Dot(oldImpWorld, t2); // t2 �������o��

			Vector3 starImp = Vector3{ oldImp1 + deltaImpulseT_desired_1, oldImp2 + deltaImpulseT_desired_2, 0.0f };
			float deltaImpulseT = starImp.length(); // ���ۂɓK�p����C���p���X

			Vector3 newImp;
			if (deltaImpulseT <= jtMaxS) // static : ���S��~
			{
				newImp = starImp;
			}
			else // slip : �����C�~���ɒ���t��
			{ 
				if (deltaImpulseT > 1e-12f) newImp = starImp * (jtMaxD / deltaImpulseT);
				else						newImp = Vector3();
			}

			Vector3 delta = newImp - Vector3{ oldImp1, oldImp2, 0.0f };

			c.m.points[i].accumImpulseT = t1 * newImp.x + t2 * newImp.y; // �ݐς͐��E�x�N�g���ŕۑ�����

			const Vector3 impulseTVector = t1 * delta.x + t2 * delta.y; // �ڐ������Ɋ|����C���p���X�x�N�g�� [tangential impulse vector]
			if (rbA && invA > 0.0f)
			{
				rbA->SetVelocity(rbA->Velocity() - impulseTVector * invA);
				rbA->SetAngularVelocity(rbA->AngularVelocity() - rbA->ApplyInvInertiaWorld(Vector3::CrossLH(rA, impulseTVector)));
				sumTor += -rbA->ApplyInvInertiaWorld(Vector3::CrossLH(rA, impulseTVector));
			}
			if (rbB && invB > 0.0f)
			{
				rbB->SetVelocity(rbB->Velocity() + impulseTVector * invB);
				rbB->SetAngularVelocity(rbB->AngularVelocity() + rbB->ApplyInvInertiaWorld(Vector3::CrossLH(rB, impulseTVector)));
			}
		}
	}
}

// ����
void PhysicsSystem::ApplyDamping(float dt)
{
	if (dt <= 0.0f) return;

	for (Collider* c : m_Colliders)
	{
		Rigidbody* rb = c->Owner()->GetComponent<Rigidbody>();
		if (!rb || !rb->IsDynamic()) continue;

		const float a = std::exp(-rb->LinDamping() * dt);
		const float b = std::exp(-rb->AngDamping() * dt);

		// ���`����
		rb->SetVelocity(rb->Velocity() * a);
		rb->SetAngularVelocity(rb->AngularVelocity() * b);
	}
}

// �ʒu�␳
void PhysicsSystem::CorrectPosition()
{
	for (auto& c : m_Contacts)
	{
		auto* rbA = c.A->Owner()->GetComponent<Rigidbody>();
		auto* rbB = c.B->Owner()->GetComponent<Rigidbody>();
		const float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
		const float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;
		if (invA + invB == 0.0f) continue;

		const Vector3 n = c.m.normal; // A��B

		float maxPen = 0.0f;
		for (int i = 0; i < c.m.count; i++)
			maxPen = std::max(maxPen, c.m.points[i].penetration); // �ő�̊ѓ��[�x���̗p
		
		// slop �Ŕ���
		const float C = std::max(0.0f, maxPen - Slop);
		if (C <= 0.0f) continue;

		// ������ƒ���
		const float s = Baumgarte * C / (invA + invB); // �t���ʔ�
		const Vector3 corrA = -n * (s * invA);
		const Vector3 corrB =  n * (s * invB);
		
		// �ʒu���f
		auto anyNonZero = [](const Vector3& v) { return v.lengthSq() > 1e-12f; };
		auto* tfA = c.A->Owner()->Transform();
		auto* tfB = c.B->Owner()->Transform();
		if (tfA && anyNonZero(corrA))
		{
			auto t = tfA->Value(); t.position += corrA; tfA->SetValue(t);
		}
		if (tfB && anyNonZero(corrB))
		{
			auto t = tfB->Value(); t.position += corrB; tfB->SetValue(t);
		}
	}
}

// ���x���f
void PhysicsSystem::IntegrationVelocity(float dt)
{
	for (Collider* c : m_Colliders)
	{
		auto* rb = c->Owner()->GetComponent<Rigidbody>();
		if (!rb || !rb->IsDynamic()) continue;

		auto* tfc = c->Owner()->Transform();

		// �ʒu���f -----
		tfc->SetWorldPosition(tfc->WorldPosition() + rb->Velocity() * dt);

		// �p���F�ւŐϕ�(rad/s) -----
		Vector3 w = rb->AngularVelocity(); // �p���x�擾
		float wlen = w.length();
		if (wlen > 1e-8f)
		{
			float theta = wlen * dt; // �p���x[rad/s] * ����[s] = �p�x[rad]
			Vector3 axis = w * (1.0f / wlen); // ��]���̒P�ʃx�N�g��
			float s = sinf(0.5f * theta), c = cosf(0.5f * theta);
			Quaternion dq(axis.x * s, axis.y * s, axis.z * s, c); // �N�H�[�^�j�I������
			tfc->SetWorldRotation((tfc->WorldRotation()).normalized() * dq); // ��]�i����j
		}

		// �����̃��[���h���� -----
		rb->UpdateInertiaWorldInvFrom(tfc->WorldRotation());
	}
}
