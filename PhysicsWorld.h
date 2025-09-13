/*
	PhysicsWorld.h
	20250523  hanaue sho
*/
#ifndef PHYSICSWORLD_H_
#define PHYSICSWORLD_H_ 

#include <iostream>
#include <vector>
#include "GameObject.h"

// --------------------------------------------------
// �}�N����`
// --------------------------------------------------


// --------------------------------------------------
// �\����
// --------------------------------------------------

// �ڐG�_���
struct Contact
{
	// �ڐG�����{�f�B�̏��
	PhysicsBody* _pBodyA = nullptr;
	PhysicsBody* _pBodyB = nullptr;

	// �ڐG�@��
	XMFLOAT3 _Normal;
	
	// �ڐG�_
	std::vector<XMFLOAT3> _ContactPoints; // �ڐG�_

	// �ѓ��[�x
	float _Penetration;

	// �����W��
	float _Restitution = 1.0f; // �����W���i�O�`�P�j
	float _StaticFriction = 0.0f; // �Ö��C�W��
	float _DynamicFriction = 0.0f; // �����C�W��

	// �ڐG�_���Ƃ̗ݐσm�[�}���C���p���X
	std::vector<float> _NormalImpulseAccum;
	// �ڐG�_���Ƃ̗ݐϖ��C�C���p���X
	std::vector<float> _TangentImpulseAccum;


public:
	Contact() = default;
	Contact(PhysicsBody* pBodyA, PhysicsBody* pBodyB, const XMFLOAT3& normal, const std::vector<XMFLOAT3>& contactPoints, float penetration, float restitution, float staticFriction, float dynamicFriction)
		: _pBodyA(pBodyA), _pBodyB(pBodyB), _Normal(normal), _ContactPoints(contactPoints), _Penetration(penetration), _Restitution(restitution), _StaticFriction(staticFriction), _DynamicFriction(dynamicFriction) 
	{
		int n = (int)_ContactPoints.size();
		_NormalImpulseAccum.assign(n, 0.0f); // �̈�m��
		_TangentImpulseAccum.assign(n, 0.0f); // �̈�m��
	}

};

// �ӂ̏��
struct Edge
{
	XMFLOAT3 _StartPoint;
	XMFLOAT3 _EndPoint;
};


// �O���錾
class Collision2d;
class BoxCollision;
class Circle;

// --------------------------------------------------
// �Q�[���I�u�W�F�N�g�}�l�[�W���[
// �V���O���g���p�^�[���Ŏ���
// --------------------------------------------------
class PhysicsWorld
{
private:
	// ---------- �V���O���g���p�^�[�� ----------
	PhysicsWorld()
	{
		std::cout << "Singleton Instance" << std::endl;
	}
	PhysicsWorld(const PhysicsWorld&) = delete;
	PhysicsWorld& operator= (const PhysicsWorld&) = delete;
	~PhysicsWorld() { m_pGameObjects.clear(); }
	static PhysicsWorld* instance; // �C���X�^���X���i�[����ÓI�ϐ�
	// ---------- �V���O���g���p�^�[�� ----------

	std::vector<GameObject*> m_pGameObjects; // �Q�[���I�u�W�F�N�g�̃x�N�g��
	std::vector<Contact> m_ContactObjects; // �R���^�N�g�I�u�W�F�N�g�̃x�N�g��
	// ���E�d�̓x�N�g��
	XMFLOAT3 m_Gravity = { 0.0f, -9.8f, 0.0f };

public:
	static PhysicsWorld* GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new PhysicsWorld();
		}
		return instance;
	}
	static void DestroyInstance()
	{
		if (instance == nullptr) return;
		delete instance;
		instance = nullptr;
	}

	void Update();

	// �d�͓K�p
	void IntegrationForce(GameObject& gameObject);
	
	// ���x����ʒu���f
	void IntegrationVelocity(GameObject& gameObject);

	// �ڐG�_�̎Z�o
	Contact ComputeContacts(Collision* pAcoll, Collision* pBcoll, const XMFLOAT3& normal, const float& penetration);
	// ReferenceEdge��IncedentEdge�̎Z�o
	Edge GetReferenceEdge(const XMFLOAT3& normal, const BoxCollision& boxA, const BoxCollision& boxB, BoxCollision& outReferenceBox, BoxCollision& outIncidentBox);
	Edge GetIncidentEdge (const XMFLOAT3& normal, const BoxCollision& incidentBox);
	// �ڐG�_�̌��̎Z�o
	std::vector<XMFLOAT3> ClipEdge(std::vector<XMFLOAT3> contactPoints, const XMFLOAT3& clipEdge, const XMFLOAT3& clipNormal);
	// �ŏI�I�ȐڐG�_�̎Z�o
	std::vector<XMFLOAT3> GetContactPoints(const XMFLOAT3& normal, const Edge& referenceEdge, const std::vector<XMFLOAT3> contactPoints);


	// �ڐG�_�̏���
	void SolveOneContact(Contact& contact, const XMFLOAT3& contactPoint, int k, int iteration);

	// �ʒu�␳
	void CorrectPosition(const Contact& contact);

	// �Q�[���I�u�W�F�N�g�̓o�^
	void RegisterGameObject(GameObject* pObject);
	// �Q�[���I�u�W�F�N�g�̍폜
	void DeleteGameObject(GameObject* pObject);

	std::vector<GameObject*> GetGameObject(const std::string& name); // ���ꂢ��H
};







#endif