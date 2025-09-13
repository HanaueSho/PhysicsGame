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
// 第一引数：管理するポインタの型（ T ）
// 第二引数：破棄時に呼ばれる削除関数（オブジェクト）の型（ Deleter ）
using PhysicsSystemPtr = std::unique_ptr<PhysicsSystem, PhysicsSystemDeleter>; // こうしないとリンカーエラーが出る

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


	// ゲームオブジェクトの登録 -----
	template < typename T > // テンプレート
	T* AddGameObject(int layer)
	{
		//GameObject* go = new T(); // こっちでできる？
		T* go = new T();
		//go->Init(); // 引数無しならここで呼んでよい。引数有りなら戻り値を使って外で呼ぶべし
		m_GameObjects[layer].push_back(go);

		return go;
	}
	/*
	※テンプレートは多用禁止※
	コンパイル時に裏で関数を必要数分（GameObjectの派生分？）用意しているだけ。
	なのでコンパイルの時間が増えるので使いどころは限るべき
	*/

	// ゲームオブジェクトの取得（先頭一つ） -----
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

	// ゲームオブジェクトの取得（全て） -----
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