/*
	PhysicsSystem.h
	20250821  hanaue sho
	物理演算
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


// 接触点の構造体
struct Contact
{
	Collider* A;
	Collider* B;
	ContactManifold m;
};

class PhysicsSystem
{
private:
	// 親シーン
	Scene* m_pScene = nullptr; // 現在のシーン
	
	// 登録コライダー
	std::vector<Collider*> m_Colliders; // 登録した Collider を管理する配列

	// 採番
	int m_NextId = 0;
	std::vector<int> m_FreeIds;	   // 解放IDの再利用
	std::vector<Collider*> m_ById; //id → collider* （逆引き用）

	// ペア集合
	uint64_t MakePairKey(uint32_t a, uint32_t b) noexcept
	{
		if (b < a) std::swap(a, b); // 順序を正規化
		return (uint64_t(a) << 32) | uint64_t(b); // 64bit の中に、小さい方を上位ビット、大きい方を下位ビットとして詰める
	}
	// 小ヘルパ
	static inline int KeyHigh(uint64_t k) { return int(uint32_t(k >> 32)); } // 上位ビットを int に変換
	static inline int KeyLow (uint64_t k) { return int(uint32_t(k & 0xffffffffu)); } // 下位ビットを int に変換

	std::unordered_set<uint64_t> m_PrevTrigger, m_CurrTrigger;
	std::unordered_set<uint64_t> m_PrevCollision, m_CurrCollision;

	// 発火キュー
	std::vector<std::pair<Collider*, Collider*>> m_TriggerEnter;
	std::vector<std::pair<Collider*, Collider*>> m_TriggerExit;
	std::vector<std::pair<Collider*, Collider*>> m_CollisionEnter;
	std::vector<std::pair<Collider*, Collider*>> m_CollisionExit;

	// 今ステップの衝突点のコンテナ
	std::vector<Contact> m_Contacts; 

public:
	PhysicsSystem(Scene& scene) : m_pScene(&scene) {}
	~PhysicsSystem() = default;

	// 初期化、終了処理
	void Init();
	void Shutdown();

	// 登録
	int RegisterCollider(Collider* c);
	void UnregisterCollider(Collider* c);

	// ステップ
	void BeginStep(float fixedDt); 
	void Step(float fixedDt); 
	void EndStep(float fixedDt);

	void DispatchEvents(); // Trigger, Collision 配信
private:
	// 力反映
	void IntegrationForce(float dt);

	// 衝突判定
	void DetermineCollision();

	// 速度解決
	void ResolveVelocity(float dt);
	
	// 減衰処理（指数減衰）
	void ApplyDamping(float dt);

	// 位置補正
	void CorrectPosition();

	// 速度反映
	void IntegrationVelocity(float dt);

};

#endif