/*
	RigidbodyComponent.h
	20250902  hanaue sho
*/
#ifndef RIGIDBODYCOMPONENT_H_
#define RIGIDBODYCOMPONENT_H_
#define NOMINMAX
#include <algorithm>
#include "Component.h"
#include "Vector3.h"
#undef min 
#undef max 

class Rigidbody : public Component
{
public:
	// �{�f�B�^�C�v -----
	// static �Փ˔���̂�
	// Kinematic �ʒu�w��̂ݔ��f
	// Dynamic ���R�����{�Փˉ����S�Ă��s��
	enum class BodyType { Static, Kinematic, Dynamic };

	// �d�͐ݒ� -----
	enum class GravityMode {Global, Custom, None};

private:
	BodyType m_BodyType = BodyType::Dynamic;

	// ���� -----
	float m_Mass	= 1.0f; // ����
	float m_InvMass = 1.0f; // �t���� �iDynamic && mass>0 �Ȃ�� 1/mass, ����ȊO��0�j
	float m_Restitution = 0.0f; // �����W��

	// ���x -----
	Vector3 m_Velocity{ 0, 0, 0 }; // ���`���x
	Vector3 m_ForceAccum{ 0, 0, 0 }; // �͂̐ϕ�
	Vector3 m_AngularVelocity{ 0, 0, 0 }; // �p���x
	Vector3 m_TorqueAccum{0, 0, 0}; // �ݐσg���N

	// ��] -----
	Vector3 m_CenterOfMassLocal{ 0, 0, 0 }; // �d�S���W
	Matrix4x4 m_InertiaLocalInv  = Matrix4x4{} * 1.0f; // ���[�J�������e���\���̋t�s��i���[�J���劵�����łǂ̎������₷������\���B�l���傫���قǁg���₷���h�j�i�p���Ɋ��Ȃ��j
	Matrix4x4 m_InertiaWorldInv {}; // ���[���h�����t�s��i���̃��[���h���ɑ΂��Ăǂꂾ�����₷������\���j�i�p���Ɋ��j
	//Vector3 m_KinematicOmega{ 0, 0, 0 }; // Kinematic �������p���x�i�܂��g��Ȃ��j

	// ���C�A���� -----
	float m_FrictionDynamic = 0.45f;   // �����C�W����
	float m_FrictionStatic  = 0.6f;   // �Î~���C�W����
	float m_LinDamping = 0.5f; // ���`�����W��[1/s]
	float m_AngDamping = 0.5f; // �p�����W��  [1/s]

	// �d�� -----
	GravityMode m_GravityMode = GravityMode::Global;  // ����F�O���[�o�����g��
	float		m_GravityScale = 1.0f;				  // �P���ʏ�A�O�����d��
	Vector3		m_CustomGravity{ 0.0f, -9.8f, 0.0f }; // Custom���Ɏg�������x

	// ���w���p -----
	void RecalcInvMass() // �t���ʂ̍Čv�Z 
	{
		if (m_BodyType == BodyType::Dynamic && m_Mass > 0) m_InvMass = 1.0f / m_Mass;
		else											   m_InvMass = 0.0f;
	}

public:
	// �Z�b�^�[
	void SetBodyType(BodyType bt) { m_BodyType = bt; RecalcInvMass(); }
	void SetMass(float m)		  { m_Mass = std::max(m, 0.0f); RecalcInvMass();} 
	void SetRestitution(float e)  { m_Restitution = e; } 

	// �Q�b�^�[
	BodyType GetBodyType() const { return m_BodyType; }	// BodyType
	bool  IsDynamic()	const { return m_BodyType == BodyType::Dynamic; }
	float Mass()		const { return m_Mass; }
	float InvMass()		const { return m_InvMass; }
	float Restitution() const { return m_Restitution; }

	const Vector3& Velocity() const { return m_Velocity; }
	void SetVelocity(const Vector3& v) {m_Velocity = v; }
	const Vector3& AngularVelocity() const { return m_AngularVelocity; }
	void SetAngularVelocity(const Vector3& v) { m_AngularVelocity = v; }

	// �����e���\�� -----
	const Matrix4x4& InertiaLocalInv() const { return m_InertiaLocalInv; }
	void SetInertiaLocalInv(const Matrix4x4& invLocal) { m_InertiaLocalInv = invLocal; }
	const Matrix4x4& InertiaWorldInv() const { return m_InertiaWorldInv; }

	// InertiaLocalInv �����݂̎p���Ő��E���W�։�
	void UpdateInertiaWorldInvFrom(const Quaternion& worldQ)
	{
		Matrix4x4 R = worldQ.ToMatrix().RotationNormalized(); // ����]�s��
		Matrix4x4 Rt = R.Transpose(); // �t�s��i����]�s��Ȃ̂œ]�u�j
		m_InertiaWorldInv = Rt * m_InertiaLocalInv * R;
	}
	// �g���N���p�����x�̊p�����v�Z����Ƃ��̃w���p�iInertiaInv��K�p������j
	Vector3 ApplyInvInertiaWorld(const Vector3& v) const
	{
		if (!IsDynamic()) return Vector3();
		return m_InertiaWorldInv.TransformNormal(v); // 3x3 �����������|����
	}

	// InertiaLocalInv �̃Z�b�g
	void ComputeBoxInertia(const Vector3& half, float mass)
	{
		float a = 2 * half.x, b = 2 * half.y, c = 2 * half.z;
		float Ix = (mass / 12.0f) * (b * b + c * c);
		float Iy = (mass / 12.0f) * (c * c + a * a);
		float Iz = (mass / 12.0f) * (a * a + b * b);
		m_InertiaLocalInv.identity();
		m_InertiaLocalInv.m[0][0] = 1.0f / Ix; // �t��
		m_InertiaLocalInv.m[1][1] = 1.0f / Iy;
		m_InertiaLocalInv.m[2][2] = 1.0f / Iz;
	}
	void ComputeSphereInertia(float radius, float mass)
	{
		const float invI = 5.0f / (2.0f * mass * radius * radius);
		m_InertiaLocalInv.identity();
		m_InertiaLocalInv.m[0][0] = invI;
		m_InertiaLocalInv.m[1][1] = invI;
		m_InertiaLocalInv.m[2][2] = invI;
	}

	// CenterOfMass �����[���h�֕ϊ�
	Vector3 WorldCOM() const
	{
		auto* tr = Owner()->GetComponent<TransformComponent>();
		return tr ? tr->WorldMatrix().TransformPoint(m_CenterOfMassLocal) : m_CenterOfMassLocal;
	}

	// ���C�A�����֌W -----
	float FrictionDynamic() const { return m_FrictionDynamic; }
	void SetFrictionDynamic(float f) { m_FrictionDynamic = f; }
	float FrictionStatic() const { return m_FrictionStatic; }
	void SetFrictionStatic(float f) { m_FrictionStatic = f; }
	float LinDamping() const { return m_LinDamping; }
	void SetLinDamping(float d) { m_LinDamping = d; }
	float AngDamping() const { return m_AngDamping; }
	void SetAngDamping(float d) { m_AngDamping = std::max(0.0f, d); }

	// �d�͊֌W -----
	void SetGravityMode(GravityMode m) { m_GravityMode = m; }
	GravityMode GravityMode() const	   { return m_GravityMode; }
	void SetGravityScale(float f) { m_GravityScale = f; }
	float GravityScale() const    { return m_GravityScale; }
	void SetCustomGravity(const Vector3& g) { m_CustomGravity = g; }
	Vector3 CustomGravity() const			{ return m_CustomGravity; }

	// Force -----
	void AddForce(const Vector3& f) { m_ForceAccum += f; } // �͂�������
	Vector3 ConsumeForces() { Vector3 f = m_ForceAccum; m_ForceAccum = { 0, 0, 0 }; return f; } // �͂�����
	void ApplyImpulse(const Vector3& P)
	{
		if (!IsDynamic()) return;
		m_Velocity += P * m_InvMass;
	}
	void ApplyImpulseAtPoint(const Vector3& P, const Vector3& worldP)
	{
		if (!IsDynamic()) return;
		m_Velocity += P * m_InvMass;
		const Vector3 r = worldP - WorldCOM();
		m_AngularVelocity += ApplyInvInertiaWorld(Vector3::Cross(r, P));
	}
	void AddTorque(const Vector3& tau) 
	{
		if (!IsDynamic()) return;
		m_TorqueAccum += tau;
	}
	// F : ���E���W�̗̓x�N�g��[N]
	// WorldP : ���̗͂������鐢�E���W��̓_[m]
	void AddForceAtPoint(const Vector3& F, const Vector3& worldP)
	{
		if (!IsDynamic()) return;

		// ���`
		m_ForceAccum += F;

		// �g���N
		const Vector3 r = worldP - WorldCOM(); // ���S����̃x�N�g��
		const Vector3 tau = Vector3::Cross(r, F); // �����Ɉˑ����ăg���N�̑傫�����ς��
		m_TorqueAccum += tau;
	}
	Vector3 ConsumeTorques() { Vector3 t = m_TorqueAccum; m_TorqueAccum = { 0, 0, 0 }; return t; } // �g���N�̏���
};

#endif