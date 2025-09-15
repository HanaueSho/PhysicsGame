/*
	ColliderComponent.h
	20250821  hanaue sho
	コライダー系のコンポーネント
*/
#ifndef COLLIDERCOMPONENT_H_
#define COLLIDERCOMPONENT_H_
#include <memory>
#include "Component.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "Collision.h"
#include "Vector3.h"
#include "manager.h"
#include "Scene.h"
#include "PhysicsSystem.h"


class Collider : public Component
{
private:
	std::unique_ptr<Collision> m_pShape = nullptr;
	AABB m_WorldAABB{}; // 広域探査で使う
	int m_Id = -1; // PhysicsSystem 採番
	bool m_IsTrigger = false;
	bool m_IsEnable = true;

	friend PhysicsSystem;
public:
	const Collision* Shape() const noexcept { return m_pShape.get(); }
		  Collision* Shape()	   noexcept { return m_pShape.get(); }
	const AABB& WorldAABB() const { return m_WorldAABB; }
	int Id() const { return m_Id; }
	void SetTrigger(bool b) { m_IsTrigger = b; }
	bool IsTrigger() const { return m_IsTrigger; }

	void UpdateWorldAABB() { m_pShape->ComputeWorldAABB(Owner()->GetComponent<TransformComponent>()->Value()); }

	// ----- 形状セッター -----
	void SetShape(std::unique_ptr<Collision> shp) { m_pShape = std::move(shp); UpdateWorldAABB(); }
	void SetBox(const Vector3& halfSize) { auto ptr = std::make_unique<BoxCollision>(halfSize); SetShape(std::move(ptr)); }
	void SetSphere(float radius) { auto ptr = std::make_unique<SphereCollision>(radius); SetShape(std::move(ptr)); }
	void SetCapsule(float radius, float height) { auto ptr = std::make_unique<CapsuleCollision>(radius, height); SetShape(std::move(ptr)); }

	// ----- ライフサイクル -----
	void OnAdded() override
	{
		//RegisterColiderの中でUpdateWorldAABBを呼ぶので先に初期化する
		SetSphere(1);
		 
		// ここで PhysicsSystem に登録（ID採番）
		m_Id = Manager::GetScene()->physicsSystem().RegisterCollider(this);
	}
	void Uninit() override
	{
		// 削除処理
		Manager::GetScene()->physicsSystem().UnregisterCollider(this);
	}

	void FixedUpdate(float dt) override
	{
		UpdateWorldAABB(); // 形状の同期
	}

	void OnTriggerEnter(class Collider* me, class Collider* other) override
	{
		//printf("当たったよ");
	}
	void OnTriggerExit(class Collider* me, class Collider* other)
	{
		//printf("離れたよ");		
	}
};

#endif