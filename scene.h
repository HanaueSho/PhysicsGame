/*
	Scene.h
	20250625  hanaue sho
*/
#ifndef SCENE_H_
#define SCENE_H_

#include <list>
#include <vector>
#include <memory>
#include "GameObject.h"

class PhysicsSystem;

struct PhysicsSystemDeleter
{
	void operator()(PhysicsSystem*) noexcept;
};
// �������F�Ǘ�����|�C���^�̌^�i T �j
// �������F�j�����ɌĂ΂��폜�֐��i�I�u�W�F�N�g�j�̌^�i Deleter �j
using PhysicsSystemPtr = std::unique_ptr<PhysicsSystem, PhysicsSystemDeleter>; // �������Ȃ��ƃ����J�[�G���[���o��

class Scene
{
protected:
	static constexpr int LAYER_COUNT = 3;
	std::list<GameObject*> m_GameObjects[LAYER_COUNT];
	PhysicsSystemPtr m_pPhysicsSystem;

public:
	Scene() = default;
	virtual ~Scene();

	virtual void Init();
	virtual void Uninit();
	virtual void Update(float dt);
	virtual void FixedUpdate(float dt);
	virtual void Draw();


	// �Q�[���I�u�W�F�N�g�̓o�^ -----
	template < typename T > // �e���v���[�g
	T* AddGameObject(int layer)
	{
		//GameObject* go = new T(); // �������łł���H
		T* go = new T();
		//go->Init(); // ���������Ȃ炱���ŌĂ�ł悢�B�����L��Ȃ�߂�l���g���ĊO�ŌĂԂׂ�
		m_GameObjects[layer].push_back(go);

		return go;
	}
	/*
	���e���v���[�g�͑��p�֎~��
	�R���p�C�����ɗ��Ŋ֐���K�v�����iGameObject�̔h�����H�j�p�ӂ��Ă��邾���B
	�Ȃ̂ŃR���p�C���̎��Ԃ�������̂Ŏg���ǂ���͌���ׂ�
	*/

	// �Q�[���I�u�W�F�N�g�̎擾�i�擪��j -----
	template < typename T >
	T* GetGameObject()
	{
		for (auto& gameObjectList : m_GameObjects)
		{
			for (auto* gameObject : gameObjectList)
			{
				T* find = dynamic_cast<T*>(gameObject);
				if (find != nullptr)
					return find;
			}
		}
		return nullptr;
	}

	// �Q�[���I�u�W�F�N�g�̎擾�i�S�āj -----
	template < typename T >
	std::vector<T*> GetGameObjects()
	{
		std::vector<T*> list;
		for (auto& gameObjectList : m_GameObjects)
		{
			for (auto* gameObject : gameObjectList)
			{
				T* find = dynamic_cast<T*>(gameObject);
				if (find != nullptr)
					list.push_back(find);
			}
		}
		return list;
	}

	PhysicsSystem& physicsSystem() { return *m_pPhysicsSystem; }
	const PhysicsSystem& physicsSystem() const { return *m_pPhysicsSystem; }
};

#endif