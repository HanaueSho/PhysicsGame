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
// マクロ定義
// --------------------------------------------------


// --------------------------------------------------
// 構造体
// --------------------------------------------------

// 接触点情報
struct Contact
{
	// 接触したボディの情報
	PhysicsBody* _pBodyA = nullptr;
	PhysicsBody* _pBodyB = nullptr;

	// 接触法線
	XMFLOAT3 _Normal;
	
	// 接触点
	std::vector<XMFLOAT3> _ContactPoints; // 接触点

	// 貫入深度
	float _Penetration;

	// 反発係数
	float _Restitution = 1.0f; // 反発係数（０～１）
	float _StaticFriction = 0.0f; // 静摩擦係数
	float _DynamicFriction = 0.0f; // 動摩擦係数

	// 接触点ごとの累積ノーマルインパルス
	std::vector<float> _NormalImpulseAccum;
	// 接触点ごとの累積摩擦インパルス
	std::vector<float> _TangentImpulseAccum;


public:
	Contact() = default;
	Contact(PhysicsBody* pBodyA, PhysicsBody* pBodyB, const XMFLOAT3& normal, const std::vector<XMFLOAT3>& contactPoints, float penetration, float restitution, float staticFriction, float dynamicFriction)
		: _pBodyA(pBodyA), _pBodyB(pBodyB), _Normal(normal), _ContactPoints(contactPoints), _Penetration(penetration), _Restitution(restitution), _StaticFriction(staticFriction), _DynamicFriction(dynamicFriction) 
	{
		int n = (int)_ContactPoints.size();
		_NormalImpulseAccum.assign(n, 0.0f); // 領域確保
		_TangentImpulseAccum.assign(n, 0.0f); // 領域確保
	}

};

// 辺の情報
struct Edge
{
	XMFLOAT3 _StartPoint;
	XMFLOAT3 _EndPoint;
};


// 前方宣言
class Collision2d;
class BoxCollision;
class Circle;

// --------------------------------------------------
// ゲームオブジェクトマネージャー
// シングルトンパターンで実装
// --------------------------------------------------
class PhysicsWorld
{
private:
	// ---------- シングルトンパターン ----------
	PhysicsWorld()
	{
		std::cout << "Singleton Instance" << std::endl;
	}
	PhysicsWorld(const PhysicsWorld&) = delete;
	PhysicsWorld& operator= (const PhysicsWorld&) = delete;
	~PhysicsWorld() { m_pGameObjects.clear(); }
	static PhysicsWorld* instance; // インスタンスを格納する静的変数
	// ---------- シングルトンパターン ----------

	std::vector<GameObject*> m_pGameObjects; // ゲームオブジェクトのベクトル
	std::vector<Contact> m_ContactObjects; // コンタクトオブジェクトのベクトル
	// 世界重力ベクトル
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

	// 重力適用
	void IntegrationForce(GameObject& gameObject);
	
	// 速度から位置反映
	void IntegrationVelocity(GameObject& gameObject);

	// 接触点の算出
	Contact ComputeContacts(Collision* pAcoll, Collision* pBcoll, const XMFLOAT3& normal, const float& penetration);
	// ReferenceEdgeとIncedentEdgeの算出
	Edge GetReferenceEdge(const XMFLOAT3& normal, const BoxCollision& boxA, const BoxCollision& boxB, BoxCollision& outReferenceBox, BoxCollision& outIncidentBox);
	Edge GetIncidentEdge (const XMFLOAT3& normal, const BoxCollision& incidentBox);
	// 接触点の候補の算出
	std::vector<XMFLOAT3> ClipEdge(std::vector<XMFLOAT3> contactPoints, const XMFLOAT3& clipEdge, const XMFLOAT3& clipNormal);
	// 最終的な接触点の算出
	std::vector<XMFLOAT3> GetContactPoints(const XMFLOAT3& normal, const Edge& referenceEdge, const std::vector<XMFLOAT3> contactPoints);


	// 接触点の処理
	void SolveOneContact(Contact& contact, const XMFLOAT3& contactPoint, int k, int iteration);

	// 位置補正
	void CorrectPosition(const Contact& contact);

	// ゲームオブジェクトの登録
	void RegisterGameObject(GameObject* pObject);
	// ゲームオブジェクトの削除
	void DeleteGameObject(GameObject* pObject);

	std::vector<GameObject*> GetGameObject(const std::string& name); // これいる？
};







#endif