/*
	PhysicsSystem.h
	20250821  hanaue sho
	�������Z
*/
#ifndef PHYSICSSYSTEM_H_
#define PHYSICSSYSTEM_H_
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <utility>
#include "ContactManifold.h"

class Scene;
class Collider;


// �ڐG�_�̍\����
struct Contact
{
	Collider* A;
	Collider* B;
	ContactManifold m;
};

class PhysicsSystem
{
private:
	// �e�V�[��
	Scene* m_pScene = nullptr; // ���݂̃V�[��
	
	// �o�^�R���C�_�[
	std::vector<Collider*> m_Colliders; // �o�^���� Collider ���Ǘ�����z��

	// �̔�
	int m_NextId = 0;
	std::vector<int> m_FreeIds;	   // ���ID�̍ė��p
	std::vector<Collider*> m_ById; //id �� collider* �i�t�����p�j

	// �y�A�W��
	uint64_t MakePairKey(uint32_t a, uint32_t b) noexcept
	{
		if (b < a) std::swap(a, b); // �����𐳋K��
		return (uint64_t(a) << 32) | uint64_t(b); // 64bit �̒��ɁA������������ʃr�b�g�A�傫���������ʃr�b�g�Ƃ��ċl�߂�
	}
	// ���w���p
	static inline int KeyHigh(uint64_t k) { return int(uint32_t(k >> 32)); } // ��ʃr�b�g�� int �ɕϊ�
	static inline int KeyLow (uint64_t k) { return int(uint32_t(k & 0xffffffffu)); } // ���ʃr�b�g�� int �ɕϊ�

	std::unordered_set<uint64_t> m_PrevTrigger, m_CurrTrigger;
	std::unordered_set<uint64_t> m_PrevCollision, m_CurrCollision;

	// ���΃L���[
	std::vector<std::pair<Collider*, Collider*>> m_TriggerEnter;
	std::vector<std::pair<Collider*, Collider*>> m_TriggerExit;
	std::vector<std::pair<Collider*, Collider*>> m_CollisionEnter;
	std::vector<std::pair<Collider*, Collider*>> m_CollisionExit;

	// ���X�e�b�v�̏Փ˓_�̃R���e�i
	std::vector<Contact> m_Contacts; 

public:
	PhysicsSystem(Scene& scene) : m_pScene(&scene) {}
	~PhysicsSystem() = default;

	// �������A�I������
	void Init();
	void Shutdown();

	// �o�^
	int RegisterCollider(Collider* c);
	void UnregisterCollider(Collider* c);

	// �X�e�b�v
	void BeginStep(float fixedDt); 
	void Step(float fixedDt); 
	void EndStep(float fixedDt);

	void DispatchEvents(); // Trigger, Collision �z�M
private:
	// �͔��f
	void IntegrationForce(float dt);

	// �Փ˔���
	void DetermineCollision();

	// ���x����
	void ResolveVelocity(float dt);
	
	// ���������i�w�������j
	void ApplyDamping(float dt);

	// �ʒu�␳
	void CorrectPosition();

	// ���x���f
	void IntegrationVelocity(float dt);

};

#endif