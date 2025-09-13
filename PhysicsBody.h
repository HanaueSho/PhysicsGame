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
	// オーナー
	GameObject* _pOwner = nullptr;

public:
	// 質量・慣性 -----
	float _Mass = 1.0f; // 質
	float _MassInverse = 0.1f; // 逆質量

	float _Inertia = 1.0f; // 慣性モーメント
	float _InertiaInverse = 1.0f; // 逆慣性

	// 速度系 -----
	XMFLOAT3 _Velocity = {0.0f, 0.0f, 0.0f}; // 速度
	float _AngularVelocity = 0.0f; // 角速度

	// 反発・摩擦係数 -----
	float _Restitution = 1.0f; // 反発係数（０～１）
	float _StaticFriction = 0.6f; // 静摩擦係数
	float _DynamicFriction = 0.4f; // 動摩擦係数

	// 減衰 -----
	float _LinearDamping = 0.01f;; // 線形減衰
	float _AngularDamping = 0.01f; // 角減衰

	// 力・トルク蓄積 -----
	XMFLOAT3 _ForceAccum = {0.0f, 0.0f, 0.0f}; // 力の合計
	float _TorqueAccum = 0.0f; // トルクの合計

	// ボディタイプ -----
	// static 衝突判定のみ
	// Kinematic 位置指定のみ反映
	// Dynamic 自然落下＋衝突解決全てを行う
	enum class BodyType {Static, Kinematic, Dynamic};
	BodyType _BodyType = BodyType::Dynamic;

	// --------------------------------------------------
	// コンストラクタ
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
	// 外力をため込む
	// --------------------------------------------------
	void AddForce(const XMFLOAT3& force)
	{
		_ForceAccum.x += force.x;
		_ForceAccum.y += force.y;
		_ForceAccum.z += force.z;
	}

	// --------------------------------------------------
	// トルクをため込む
	// --------------------------------------------------
	void AddTorque(float torque)
	{
		_TorqueAccum += torque;
	}

	// --------------------------------------------------
	// 速度に直接インパルスを与える
	// impulse: 法線方向インパルスベクトル
	// contactVector: インパルスを加える接触点位置ベクトル contactPoint - center
	// ※どちらもワールド座標系
	// --------------------------------------------------
	void ApplyImpulse(const XMFLOAT3& impulse, const XMFLOAT3& contactVector)
	{
		// 水平速度に追加 v = impulse * massInverse
		_Velocity.x += impulse.x * _MassInverse;
		_Velocity.y += impulse.y * _MassInverse;
		_Velocity.z += impulse.z * _MassInverse;
	
		// 回転速度に追加 ω = (vect * impulse) * inertiaInverse
		float crossZ = contactVector.x * impulse.y - contactVector.y * impulse.x; // 外積
		_AngularVelocity += crossZ * _InertiaInverse;
	}


};



#endif