/*
	PhysicsBody.h
	20250522 hanaue sho
*/
#ifndef PHYSICSBODY_H_
#define PHYSICSBODY_H_
#include <DirectXMath.h>
using namespace DirectX;

class GameObject;

struct PhysicsBody
{
public:
	// �I�[�i�[
	GameObject* _pOwner = nullptr;

public:
	// ���ʁE���� -----
	float _Mass = 1.0f; // ��
	float _MassInverse = 0.1f; // �t����

	float _Inertia = 1.0f; // �������[�����g
	float _InertiaInverse = 1.0f; // �t����

	// ���x�n -----
	XMFLOAT3 _Velocity = {0.0f, 0.0f, 0.0f}; // ���x
	float _AngularVelocity = 0.0f; // �p���x

	// �����E���C�W�� -----
	float _Restitution = 1.0f; // �����W���i�O�`�P�j
	float _StaticFriction = 0.6f; // �Ö��C�W��
	float _DynamicFriction = 0.4f; // �����C�W��

	// ���� -----
	float _LinearDamping = 0.01f;; // ���`����
	float _AngularDamping = 0.01f; // �p����

	// �́E�g���N�~�� -----
	XMFLOAT3 _ForceAccum = {0.0f, 0.0f, 0.0f}; // �͂̍��v
	float _TorqueAccum = 0.0f; // �g���N�̍��v

	// �{�f�B�^�C�v -----
	// static �Փ˔���̂�
	// Kinematic �ʒu�w��̂ݔ��f
	// Dynamic ���R�����{�Փˉ����S�Ă��s��
	enum class BodyType {Static, Kinematic, Dynamic};
	BodyType _BodyType = BodyType::Dynamic;

	// --------------------------------------------------
	// �R���X�g���N�^
	// --------------------------------------------------
	PhysicsBody() = default;
	PhysicsBody(GameObject* pOwner) : _pOwner(pOwner) {}
	PhysicsBody(GameObject* pOwner, float mass, float inertia, BodyType bodyType) : _pOwner(pOwner), _Mass(mass), _Inertia(inertia), _BodyType(bodyType)
	{
		_MassInverse = 1.0f / mass;
		_InertiaInverse = 1.0f / inertia;
	}


	GameObject* GetOwner() const { return _pOwner; }

	// --------------------------------------------------
	// �O�͂����ߍ���
	// --------------------------------------------------
	void AddForce(const XMFLOAT3& force)
	{
		_ForceAccum.x += force.x;
		_ForceAccum.y += force.y;
		_ForceAccum.z += force.z;
	}

	// --------------------------------------------------
	// �g���N�����ߍ���
	// --------------------------------------------------
	void AddTorque(float torque)
	{
		_TorqueAccum += torque;
	}

	// --------------------------------------------------
	// ���x�ɒ��ڃC���p���X��^����
	// impulse: �@�������C���p���X�x�N�g��
	// contactVector: �C���p���X��������ڐG�_�ʒu�x�N�g�� contactPoint - center
	// ���ǂ�������[���h���W�n
	// --------------------------------------------------
	void ApplyImpulse(const XMFLOAT3& impulse, const XMFLOAT3& contactVector)
	{
		// �������x�ɒǉ� v = impulse * massInverse
		_Velocity.x += impulse.x * _MassInverse;
		_Velocity.y += impulse.y * _MassInverse;
		_Velocity.z += impulse.z * _MassInverse;
	
		// ��]���x�ɒǉ� �� = (vect * impulse) * inertiaInverse
		float crossZ = contactVector.x * impulse.y - contactVector.y * impulse.x; // �O��
		_AngularVelocity += crossZ * _InertiaInverse;
	}


};



#endif