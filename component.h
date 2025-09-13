/*
	Component.h
	20250813  hanaue sho
	�w�Z�̂������
*/
#ifndef COMPONENT_H_
#define COMPONENT_H_

class GameObject; // �O���錾

class Component
{
protected:
	GameObject* m_pOwner = nullptr;

public:
	Component() = default;
	virtual ~Component() = default;

	// ���L�҃A�N�Z�X�i�ǂݎ���pAPI�j
	GameObject*			Owner()		  noexcept  { return m_pOwner; }
	const GameObject*	Owner() const noexcept	{ return m_pOwner; }

	// ----- ���C�t�T�C�N�� -----
	virtual void OnAdded() {}		// AddComponent��
	virtual void Init() {}			// GameObject.Init()��
	virtual void Uninit() {}		// GameObject�j����

	// ----- �X�V -----
	virtual void FixedUpdate(float dt) {}	// �Œ�X�V�i�������Z�n�j
	virtual void Update(float dt) {}		// �ʏ�X�V
	virtual void Draw() {}					// �`��

	// ----- �f�B�X�p�b�` -----
	virtual void OnTriggerEnter(class Collider* me, class Collider* other) {}
	virtual void OnTriggerExit (class Collider* me, class Collider* other) {}

	virtual void OnCollisionEnter(class Collider* me, class Collider* other) {}
	virtual void OnCollisionExit (class Collider* me, class Collider* other) {}


	// �R�s�[�A���[�u�͌����֎~�i���L�҂ƃ��C�t�T�C�N�������ނ��߁j
	Component(const Component&) = delete;
	Component& operator =(const Component&) = delete;
	Component(Component&&) = delete;
	Component& operator =(const Component&&) = delete;

	// owner ��ݒ�ł���̂� GameObject �����Ɍ���
	friend class GameObject;
};

#endif