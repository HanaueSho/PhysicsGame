/*
	GameObject.h
	20250813  hanaue sho
	Component�u���։����i�w�b�_���֊����j
*/
#ifndef GAMEOBJECT_H_
#define GAMEOBJECT_H_
#include <vector>
#include <memory>
#include <type_traits>
#include <utility>
#include "Component.h"
#include "TransformComponent.h"


class GameObject
{
protected:
	bool m_IsDestroy = false; // �j���t���O
	std::vector<std::unique_ptr<Component>> m_Components;

public:
	GameObject()
	{
		// �K�{�FTransform ���P�����t�^�i�d���t�^�͂��Ȃ��݌v�j
		AddComponent<TransformComponent>();
	}
	virtual ~GameObject() = default;

	// ----- ���C�t�T�C�N�� ----- �i�t�^�ςݑS�R���|�[�l���g�ɔz�M�j
	virtual void Init()					{ for (auto& c : m_Components) c->Init(); }
	virtual void Uninit()				{ for (auto& c : m_Components) c->Uninit(); }
	virtual void FixedUpdate(float dt)	{ for (auto& c : m_Components) c->FixedUpdate(dt); }
	virtual void Update(float dt)		{ for (auto& c : m_Components) c->Update(dt); }
	virtual void Draw()					{ for (auto& c : m_Components) c->Draw(); }

	// ----- �j���t���O -----
	void RequestDestroy()			noexcept { m_IsDestroy = true; }
	bool IsDestroyRequested() const noexcept { return m_IsDestroy; }

	// ----- �R���|�l���g�Ǘ� -----
	// �������� Owner �ݒ�� OnAdd ���s����
	template<class T, class... Args> 
	T* AddComponent(Args&&... args)
	{
		static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
		auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
		T* raw = ptr.get();
		// friend �w��ɂ�� GameObject ������ owner ��ݒ�\
		raw->m_pOwner = this;
		m_Components.emplace_back(std::move(ptr));
		raw->OnAdded(); // AddComponent ���ɌĂԊ֐�
		return raw;
	}

	// ��const�ŁF���������\�� T* ��Ԃ�
	template<class T>
	T* GetComponent() noexcept 
	{
		for (auto& c : m_Components)
			if (auto p = dynamic_cast<T*>(c.get())) return p;
		return nullptr;
	}

	// const�ŁF�ǂݎ���p�� const T* ��Ԃ�
	template<class T>
	const T* GetComponent() const noexcept
	{
		for (auto& c : m_Components)
			if (auto p = dynamic_cast<const T*>(c.get())) return p;
		return nullptr;
	}

	// �R���|�[�l���g�̎擾�i�w���p�j
	template<class F> // callable �� F 
	void ForEachComponent(F&& f) // F&& : �]���Q�Ɓilvalue��rvalue���󂯎��閜�\�Ȏ󂯌��H�H�j
	{
		std::vector<Component*> snapshot; 
		snapshot.reserve(m_Components.size());
		for (auto& up : m_Components) if (up) snapshot.push_back(up.get()); // �X�i�b�v�V���b�g�i�R�s�[�j�����
		for (Component* c : snapshot) if (c) std::forward<F>(f) (c); //  f(c)... �֐� F ������ c �ŌĂ� �i�H�j
	}


	// Transform �͕K���P���݁i�R���X�g���N�^�ŕt�^�j
	TransformComponent* Transform()			noexcept { return GetComponent<TransformComponent>(); }
	const TransformComponent* Transform() const	noexcept { return GetComponent<TransformComponent>(); }

};

#endif