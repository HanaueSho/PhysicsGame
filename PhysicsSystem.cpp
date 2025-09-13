/*
	PhysicsSystem.cpp
	20250821  hanaue sho
	物理演算
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
	Vector3 Gravity = { 0.0f, -9.8f, 0.0f }; // 必要なら０
	const float Slop = 0.005f;				 // 許容量（大きすぎるとめり込むよ）
	const float RestThreshold = 0.2f;		 // 反発バイアス許容量
	const float Baumgarte = 0.2f;			 // 位置補正の係数

	// クランプ
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
	m_ById[id] = c; // 逆引き用に登録

	c->UpdateWorldAABB();
	m_Colliders.push_back(c);
	return id;
}

void PhysicsSystem::UnregisterCollider(Collider* c)
{
	if (!c || c->m_Id < 0) return;

	// ペア集合から除去
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

	// 逆引き
	m_ById[id] = nullptr;
	m_FreeIds.push_back(id);

	// リストから除外
	auto it = std::find(m_Colliders.begin(), m_Colliders.end(), c);
	if (it != m_Colliders.end()) { *it = m_Colliders.back(); m_Colliders.pop_back(); }
	c->m_Id = -1;
}

// ステップの始まり
// スクリプトの FixedUpdate を先に実行
void PhysicsSystem::BeginStep(float fixedDt)
{
	m_CurrTrigger.clear();
	m_CurrCollision.clear();
	m_Contacts.clear();

	// ----- 外力→速度（重力など）-----
	IntegrationForce(fixedDt);

	// ステップの最初に WorldAABB を更新
	for (Collider* c : m_Colliders)
		c->UpdateWorldAABB();
}

void PhysicsSystem::Step(float fixedDt)
{
	// ----- 衝突判定 -----
	DetermineCollision();

	// ----- 速度解決 -----
	for (int i = 0; i < 8; i++) { ResolveVelocity(fixedDt); /*printf("---------- resolve : %d\n", i);*/ }
}

void PhysicsSystem::EndStep(float fixedDt)
{
	// ----- 減衰 -----
	ApplyDamping(fixedDt);

	// ----- 速度積分 -----
	IntegrationVelocity(fixedDt);

	// ----- 位置補正 -----
	for (int i = 0; i < 4; i++)	CorrectPosition();


	auto diffSets = [&](auto& prev, auto& curr, auto& outEnter, auto& outExit)
		{
			// Enter : Curr にあって Prev にない
			for (auto k: curr) 
				if (!prev.count(k))
				{
					outEnter.emplace_back(m_ById[KeyHigh(k)], m_ById[KeyLow(k)]); // Enter にいれる
				}
			// Exit : Prev にあって Curr にない
			for (auto k : prev) 
				if (!curr.count(k))
				{
					outExit.emplace_back(m_ById[KeyHigh(k)], m_ById[KeyLow(k)]); // Exit にいれる
				}
			prev.swap(curr); curr.clear();
		};

	diffSets(m_PrevTrigger  , m_CurrTrigger  , m_TriggerEnter  , m_TriggerExit);
	diffSets(m_PrevCollision, m_CurrCollision, m_CollisionEnter, m_CollisionExit);

	// ----- ディスパッチ処理 -----
	DispatchEvents(); 
}

void PhysicsSystem::DispatchEvents()
{
	// OnTriggerEnte, Exit の呼び出し
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
					}); // ”ラムダ”を渡している
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

	// 後始末
	m_TriggerEnter.clear();
	m_TriggerExit.clear();
	m_CollisionEnter.clear();
	m_CollisionExit.clear();
}

// 力反映
void PhysicsSystem::IntegrationForce(float dt)
{
	for (Collider* c : m_Colliders)
	{
		auto* rb = c->Owner()->GetComponent<Rigidbody>();
		if (!rb || !rb->IsDynamic()) continue;

		// ここで InertiaWorldInv の更新 -----
		rb->UpdateInertiaWorldInvFrom(c->Owner()->GetComponent<TransformComponent>()->WorldRotation());

		// 重力加速度の決定
		Vector3 gravity{ 0.0f, 0.0f, 0.0f };
		switch (rb->GravityMode())
		{
		case Rigidbody::GravityMode::Global: // グローバル重力
			gravity = Gravity * rb->GravityScale();
			break;
		case Rigidbody::GravityMode::Custom: // カスタム重力
			gravity = rb->CustomGravity();
			break;
		case Rigidbody::GravityMode::None: // 無重力
			gravity = {0.0f, 0.0f, 0.0f};
			break;
		}

		// a = g + F/m
		Vector3 force = rb->ConsumeForces();
		Vector3 acc = gravity + force * rb->InvMass();
		rb->SetVelocity(rb->Velocity() + acc * dt);

		// トルク反映
		// ω += InertiaWorld^-1 * t * dt
		Vector3 tau = rb->ConsumeTorques();
		Vector3 omega = rb->ApplyInvInertiaWorld(tau) * dt;
		rb->SetAngularVelocity(rb->AngularVelocity() + omega);
	}
}

// 衝突判定
void PhysicsSystem::DetermineCollision()
{
	// 当たり判定
	const int n = (int)m_Colliders.size();
	for (int ia = 0; ia < n; ia++)
	{
		// Collider 取得
		Collider* A = m_Colliders[ia];
		const Collision* shapeA = A->Shape();
		if (!shapeA) continue;

		const Transform& transA = A->Owner()->GetComponent<TransformComponent>()->Value();

		for (int ib = ia + 1; ib < n; ib++)
		{
			Collider* B = m_Colliders[ib];

			// ブロード（AABB）
			if (!A->WorldAABB().isOverlap(B->WorldAABB())) continue; // AABB で当たっているか大まかに判定

			// ナロー
			const Collision* shapeB = B->Shape();
			if (!shapeB) continue;

			const Transform& transB = B->Owner()->GetComponent<TransformComponent>()->Value();

			ContactManifold m; 
			if (!shapeA->isOverlap(transA, *shapeB, transB, m, 0.0f)) continue;

			if (A->IsTrigger() || B->IsTrigger()) m_CurrTrigger.insert(MakePairKey(A->Id(), B->Id()));
			else // どっちも IsTrigger が off だったら Collision イベントへ			  
			{
				m_CurrCollision.insert(MakePairKey(A->Id(), B->Id()));
				if (m.touching && m.count > 0)
					m_Contacts.push_back({ A, B, m });
			}
		}
	}
}

// 速度解決
// 逐次インパルス法（接触、摩擦、ジョイントなどの速度拘束で即座に速度を更新する手法）
void PhysicsSystem::ResolveVelocity(float dt)
{
	for (auto& c : m_Contacts)
	{
		auto* rbA = c.A->Owner()->GetComponent<Rigidbody>();
		auto* rbB = c.B->Owner()->GetComponent<Rigidbody>();

		const float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f; // 逆質量取得
		const float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f; // 逆質量取得
		if (invA + invB == 0.0f) continue; // 両方固定

		const Vector3 n = c.m.normal; // A→B

		// 反発係数
		const float eCandidate = std::max(rbA ? rbA->Restitution() : 0.0f,
										  rbB ? rbB->Restitution() : 0.0f);

		// 摩擦係数（現在はRigidbody）
		const float fricDynamicA = rbA ? rbA->FrictionDynamic() : 0.0f;
		const float fricDynamicB = rbB ? rbB->FrictionDynamic() : 0.0f;
		const float fricStaticA = rbA ? rbA->FrictionStatic() : 0.0f;
		const float fricStaticB = rbB ? rbB->FrictionStatic() : 0.0f;
		auto Mix = [](float a, float b) {return (a > 0.0f && b > 0.0f) ? sqrtf(a * b) : std::max(a, b); };
		float fricD = Mix(fricDynamicA, fricDynamicB);
		float fricS = Mix(fricStaticA, fricStaticB);
		fricD = std::max(0.0f, fricD);
		fricS = std::max(0.0f, std::max(fricS, fricD)); // fricS >= fricD

		// ----- 角項の小ヘルパ -----
		// d : 評価方向（法線または接線）
		// 接触点に方向ｄのインパルスを 1[N・s]入れたときの「角運動による逆加速度寄与」をスカラーで返す
		// 接触点速度(v = ω×r)にインパルス(P = d * j)を入れたときの角速度変化(Δω = I^-1(r×P))を接触点速度へ戻した寄与((I^-1(r×d))×r)をｄで射影したもの（？）
		auto AngTerm = [](Rigidbody* rb, const Vector3& r, const Vector3& d) -> float
			{
				// n・[(I^-1 (r×d))×r] = dot(d, (r×d))×r)を展開してもいいが可読性重視でこちら
				return  Vector3::Dot(d, Vector3::CrossLH(rb->ApplyInvInertiaWorld(Vector3::CrossLH(r, d)), r));
			};

		
		// ======== PHASE 1: 全接点の「法線」だけ解く =========
		// 各接触点について法線インパルス
		Vector3 sumTor = Vector3();
		for (int i = 0; i < c.m.count; i++)
		{
			// 接触点と COM 差分を取る
			const Vector3 p = (c.m.points[i].pointOnA + c.m.points[i].pointOnB) * 0.5f; //接触点の中点を求める
			const Vector3 pA = c.m.points[i].pointOnA;
			const Vector3 pB = c.m.points[i].pointOnB;
			const Vector3 xA = rbA ? rbA->WorldCOM() : Vector3(); // 重心座標
			const Vector3 xB = rbB ? rbB->WorldCOM() : Vector3(); // 重心座標
			const Vector3 rA = p - xA; // COM からのベクトル（相対位置）
			const Vector3 rB = p - xB; // COM からのベクトル（相対位置）

			// 速度（角成分込み）
			const Vector3 vA = rbA ? rbA->Velocity()		: Vector3{};
			const Vector3 wA = rbA ? rbA->AngularVelocity() : Vector3{};
			const Vector3 vB = rbB ? rbB->Velocity()		: Vector3{};
			const Vector3 wB = rbB ? rbB->AngularVelocity() : Vector3{};

			// 接触点の相対速度 vRel = (vB + wB×rB) - (vA + wA×rA) [relative velocity]
			const Vector3 vRel = (vB + Vector3::CrossLH(wB, rB)) - (vA + Vector3::CrossLH(wA, rA)); // 相対速度（Aから見たBの相対速度）
			const float relVelN = Vector3::Dot(vRel, n); // 法線方向の相対速度 [normal relative velocity]
			if (relVelN > 0.0f && c.m.points[i].penetration <= Slop) continue; // 離れていく方向 && めり込みが無い なので不要


			// ----- 法線インパルス（角項込みの有効質量） -----
			// 有効質量の分母（法線）
			float denomN = invA + invB; // 角項込みの法線方向の有効質量の逆 [normal effective mass]
			if (rbA && invA > 0.0f) denomN += AngTerm(rbA, rA, n); // 接触点速度の“ｄ方向成分”がどれだけ変わるかを表す係数↓
			if (rbB && invB > 0.0f) denomN += AngTerm(rbB, rB, n); // 分母 k = (invA + invB + AngTermA + AngTermB)
			if (denomN < 1e-12f) continue;

			// 位置バイアス
			const float pen = c.m.points[i].penetration;
			const float bias_pos = pen > Slop ? Baumgarte * (pen - Slop) / dt : 0.0f;
			// 反発バイアス
			const float bias_rest = relVelN < -RestThreshold ? (-eCandidate * relVelN) : 0.0f; // ここで反発処理が含まれる

			// 増分インパルス（インパルス量）
			float deltaImpulseN = -(relVelN + bias_pos + bias_rest) / denomN;

			// 累積クランプ
			float old = c.m.points[i].accumN;
			c.m.points[i].accumN = std::max(0.0f, old + deltaImpulseN);
			deltaImpulseN = c.m.points[i].accumN - old; // 実際に適用する増分

			const Vector3 impulseVector = n * deltaImpulseN; // ベクトル化したインパルス [normal impulse vector]

			// v と ω に即時適用 -----
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
		
		// ======== PHASE 2: 全接点の「摩擦」だけ解く =========
		// 各接触点について摩擦
		for (int i = 0; i < c.m.count; i++)
		{
			// 接触点と COM 差分を取る
			const Vector3 p = (c.m.points[i].pointOnA + c.m.points[i].pointOnB) * 0.5f; //接触点の中点を求める
			const Vector3 pA = c.m.points[i].pointOnA;
			const Vector3 pB = c.m.points[i].pointOnB;
			const Vector3 xA = rbA ? rbA->WorldCOM() : Vector3(); // 重心座標
			const Vector3 xB = rbB ? rbB->WorldCOM() : Vector3(); // 重心座標
			const Vector3 rA = p - xA; // COM からのベクトル（相対位置）
			const Vector3 rB = p - xB; // COM からのベクトル（相対位置）

			// ----- 動摩擦力（法線更新後の相対速度で評価） -----
			const Vector3 vA2 = rbA ? rbA->Velocity() : Vector3{};
			const Vector3 wA2 = rbA ? rbA->AngularVelocity() : Vector3{};
			const Vector3 vB2 = rbB ? rbB->Velocity() : Vector3{};
			const Vector3 wB2 = rbB ? rbB->AngularVelocity() : Vector3{};
			const Vector3 vRel2 = (vB2 + Vector3::CrossLH(wB2, rB)) - (vA2 + Vector3::CrossLH(wA2, rA)); // 相対速度

			// ----- 希望する接線のインパルス -----
			Vector3 t1;
			if (fabsf(n.x) > 0.57735f) t1 = Vector3{ -n.y, n.x, 0.0f };
			else					   t1 = Vector3{ 0.0f, -n.z, n.y };
			t1 = t1.normalized();
			Vector3 t2 = Vector3::CrossLH(n, t1); // 接線方向（２軸目）

			// 有効質量の分母
			float denomT1 = invA + invB; // 接線方向の有効質量の分母 [tangential impulse mass]
			if (rbA && invA > 0.0f) denomT1 += AngTerm(rbA, rA, t1);
			if (rbB && invB > 0.0f) denomT1 += AngTerm(rbB, rB, t1);
			if (denomT1 < 1e-12f) continue;
			float denomT2 = invA + invB; // 接線方向の有効質量の分母 [tangential impulse mass]
			if (rbA && invA > 0.0f) denomT2 += AngTerm(rbA, rA, t2);
			if (rbB && invB > 0.0f) denomT2 += AngTerm(rbB, rB, t2);
			if (denomT2 < 1e-12f) continue;

			// 望ましい増分（静止摩擦ターゲット vt' = 0）
			float v_t1 = Vector3::Dot(vRel2, t1);
			float v_t2 = Vector3::Dot(vRel2, t2);
			float deltaImpulseT_desired_1 = -(v_t1) / denomT1; // [tangential impulse scalar]
			float deltaImpulseT_desired_2 = -(v_t2) / denomT2; // [tangential impulse scalar]

			// ----- Coulomb 円錐（半径 = μ * jn）でクランプ -----
			float jtMaxS = fricS * c.m.points[i].accumN; // 静止摩擦半径　μ * jn （累積）
			float jtMaxD = fricD * c.m.points[i].accumN; // 動摩擦半径　μ * jn （累積）
			if (jtMaxD <= 0.0f) continue; // 法線反力がない≒摩擦が無い

			Vector3 oldImpWorld = c.m.points[i].accumImpulseT; // ワールド累積
			float oldImp1 = Vector3::Dot(oldImpWorld, t1); // t1 成分取り出し
			float oldImp2 = Vector3::Dot(oldImpWorld, t2); // t2 成分取り出し

			Vector3 starImp = Vector3{ oldImp1 + deltaImpulseT_desired_1, oldImp2 + deltaImpulseT_desired_2, 0.0f };
			float deltaImpulseT = starImp.length(); // 実際に適用するインパルス

			Vector3 newImp;
			if (deltaImpulseT <= jtMaxS) // static : 完全停止
			{
				newImp = starImp;
			}
			else // slip : 動摩擦円錐に張り付け
			{ 
				if (deltaImpulseT > 1e-12f) newImp = starImp * (jtMaxD / deltaImpulseT);
				else						newImp = Vector3();
			}

			Vector3 delta = newImp - Vector3{ oldImp1, oldImp2, 0.0f };

			c.m.points[i].accumImpulseT = t1 * newImp.x + t2 * newImp.y; // 累積は世界ベクトルで保存する

			const Vector3 impulseTVector = t1 * delta.x + t2 * delta.y; // 接線方向に掛けるインパルスベクトル [tangential impulse vector]
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

// 減衰
void PhysicsSystem::ApplyDamping(float dt)
{
	if (dt <= 0.0f) return;

	for (Collider* c : m_Colliders)
	{
		Rigidbody* rb = c->Owner()->GetComponent<Rigidbody>();
		if (!rb || !rb->IsDynamic()) continue;

		const float a = std::exp(-rb->LinDamping() * dt);
		const float b = std::exp(-rb->AngDamping() * dt);

		// 線形減衰
		rb->SetVelocity(rb->Velocity() * a);
		rb->SetAngularVelocity(rb->AngularVelocity() * b);
	}
}

// 位置補正
void PhysicsSystem::CorrectPosition()
{
	for (auto& c : m_Contacts)
	{
		auto* rbA = c.A->Owner()->GetComponent<Rigidbody>();
		auto* rbB = c.B->Owner()->GetComponent<Rigidbody>();
		const float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
		const float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;
		if (invA + invB == 0.0f) continue;

		const Vector3 n = c.m.normal; // A→B

		float maxPen = 0.0f;
		for (int i = 0; i < c.m.count; i++)
			maxPen = std::max(maxPen, c.m.points[i].penetration); // 最大の貫入深度を採用
		
		// slop で判定
		const float C = std::max(0.0f, maxPen - Slop);
		if (C <= 0.0f) continue;

		// じわっと直す
		const float s = Baumgarte * C / (invA + invB); // 逆質量比
		const Vector3 corrA = -n * (s * invA);
		const Vector3 corrB =  n * (s * invB);
		
		// 位置反映
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

// 速度反映
void PhysicsSystem::IntegrationVelocity(float dt)
{
	for (Collider* c : m_Colliders)
	{
		auto* rb = c->Owner()->GetComponent<Rigidbody>();
		if (!rb || !rb->IsDynamic()) continue;

		auto* tfc = c->Owner()->Transform();

		// 位置反映 -----
		tfc->SetWorldPosition(tfc->WorldPosition() + rb->Velocity() * dt);

		// 姿勢：ωで積分(rad/s) -----
		Vector3 w = rb->AngularVelocity(); // 角速度取得
		float wlen = w.length();
		if (wlen > 1e-8f)
		{
			float theta = wlen * dt; // 角速度[rad/s] * 時間[s] = 角度[rad]
			Vector3 axis = w * (1.0f / wlen); // 回転軸の単位ベクトル
			float s = sinf(0.5f * theta), c = cosf(0.5f * theta);
			Quaternion dq(axis.x * s, axis.y * s, axis.z * s, c); // クォータニオン生成
			tfc->SetWorldRotation((tfc->WorldRotation()).normalized() * dq); // 回転（左手）
		}

		// 慣性のワールド同期 -----
		rb->UpdateInertiaWorldInvFrom(tfc->WorldRotation());
	}
}
