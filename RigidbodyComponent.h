/*
	RigidbodyComponent.h
	20250902  hanaue sho
*/
#ifndef RIGIDBODYCOMPONENT_H_
#define RIGIDBODYCOMPONENT_H_
#define NOMINMAX
#include <algorithm>
#include "Component.h"
#include "Vector3.h"
#undef min 
#undef max 

class Rigidbody : public Component
{
public:
	// ボディタイプ -----
	// static 衝突判定のみ
	// Kinematic 位置指定のみ反映
	// Dynamic 自然落下＋衝突解決全てを行う
	enum class BodyType { Static, Kinematic, Dynamic };

	// 重力設定 -----
	enum class GravityMode {Global, Custom, None};

private:
	BodyType m_BodyType = BodyType::Dynamic;

	// 質量 -----
	float m_Mass	= 1.0f; // 質量
	float m_InvMass = 1.0f; // 逆質量 （Dynamic && mass>0 ならば 1/mass, それ以外は0）
	float m_Restitution = 0.0f; // 反発係数

	// 速度 -----
	Vector3 m_Velocity{ 0, 0, 0 }; // 線形速度
	Vector3 m_ForceAccum{ 0, 0, 0 }; // 力の積分
	Vector3 m_AngularVelocity{ 0, 0, 0 }; // 角速度
	Vector3 m_TorqueAccum{0, 0, 0}; // 累積トルク

	// 回転 -----
	Vector3 m_CenterOfMassLocal{ 0, 0, 0 }; // 重心座標
	Matrix4x4 m_InertiaLocalInv  = Matrix4x4{} * 1.0f; // ローカル慣性テンソルの逆行列（ローカル主慣性軸でどの軸が回りやすいかを表す。値が大きいほど“回りやすい”）（姿勢に寄らない）
	Matrix4x4 m_InertiaWorldInv {}; // ワールド慣性逆行列（今のワールド軸に対してどれだけ回りやすいかを表す）（姿勢に寄る）
	//Vector3 m_KinematicOmega{ 0, 0, 0 }; // Kinematic 見かけ角速度（まだ使わない）

	// 摩擦、減衰 -----
	float m_FrictionDynamic = 0.45f;   // 動摩擦係数μ
	float m_FrictionStatic  = 0.6f;   // 静止摩擦係数μ
	float m_LinDamping = 0.5f; // 線形減衰係数[1/s]
	float m_AngDamping = 0.5f; // 角減衰係数  [1/s]

	// 重力 -----
	GravityMode m_GravityMode = GravityMode::Global;  // 既定：グローバルを使う
	float		m_GravityScale = 1.0f;				  // １＝通常、０＝無重力
	Vector3		m_CustomGravity{ 0.0f, -9.8f, 0.0f }; // Custom時に使う加速度

	// 小ヘルパ -----
	void RecalcInvMass() // 逆質量の再計算 
	{
		if (m_BodyType == BodyType::Dynamic && m_Mass > 0) m_InvMass = 1.0f / m_Mass;
		else											   m_InvMass = 0.0f;
	}

public:
	// セッター
	void SetBodyType(BodyType bt) { m_BodyType = bt; RecalcInvMass(); }
	void SetMass(float m)		  { m_Mass = std::max(m, 0.0f); RecalcInvMass();} 
	void SetRestitution(float e)  { m_Restitution = e; } 

	// ゲッター
	BodyType GetBodyType() const { return m_BodyType; }	// BodyType
	bool  IsDynamic()	const { return m_BodyType == BodyType::Dynamic; }
	float Mass()		const { return m_Mass; }
	float InvMass()		const { return m_InvMass; }
	float Restitution() const { return m_Restitution; }

	const Vector3& Velocity() const { return m_Velocity; }
	void SetVelocity(const Vector3& v) {m_Velocity = v; }
	const Vector3& AngularVelocity() const { return m_AngularVelocity; }
	void SetAngularVelocity(const Vector3& v) { m_AngularVelocity = v; }

	// 慣性テンソル -----
	const Matrix4x4& InertiaLocalInv() const { return m_InertiaLocalInv; }
	void SetInertiaLocalInv(const Matrix4x4& invLocal) { m_InertiaLocalInv = invLocal; }
	const Matrix4x4& InertiaWorldInv() const { return m_InertiaWorldInv; }

	// InertiaLocalInv を現在の姿勢で世界座標へ回す
	void UpdateInertiaWorldInvFrom(const Quaternion& worldQ)
	{
		Matrix4x4 R = worldQ.ToMatrix().RotationNormalized(); // 純回転行列
		Matrix4x4 Rt = R.Transpose(); // 逆行列（純回転行列なので転置）
		m_InertiaWorldInv = Rt * m_InertiaLocalInv * R;
	}
	// トルク→角加速度の角項を計算するときのヘルパ（InertiaInvを適用させる）
	Vector3 ApplyInvInertiaWorld(const Vector3& v) const
	{
		if (!IsDynamic()) return Vector3();
		return m_InertiaWorldInv.TransformNormal(v); // 3x3 部分だけを掛ける
	}

	// InertiaLocalInv のセット
	void ComputeBoxInertia(const Vector3& half, float mass)
	{
		float a = 2 * half.x, b = 2 * half.y, c = 2 * half.z;
		float Ix = (mass / 12.0f) * (b * b + c * c);
		float Iy = (mass / 12.0f) * (c * c + a * a);
		float Iz = (mass / 12.0f) * (a * a + b * b);
		m_InertiaLocalInv.identity();
		m_InertiaLocalInv.m[0][0] = 1.0f / Ix; // 逆数
		m_InertiaLocalInv.m[1][1] = 1.0f / Iy;
		m_InertiaLocalInv.m[2][2] = 1.0f / Iz;
	}
	void ComputeSphereInertia(float radius, float mass)
	{
		const float invI = 5.0f / (2.0f * mass * radius * radius);
		m_InertiaLocalInv.identity();
		m_InertiaLocalInv.m[0][0] = invI;
		m_InertiaLocalInv.m[1][1] = invI;
		m_InertiaLocalInv.m[2][2] = invI;
	}

	// CenterOfMass をワールドへ変換
	Vector3 WorldCOM() const
	{
		auto* tr = Owner()->GetComponent<TransformComponent>();
		return tr ? tr->WorldMatrix().TransformPoint(m_CenterOfMassLocal) : m_CenterOfMassLocal;
	}

	// 摩擦、減衰関係 -----
	float FrictionDynamic() const { return m_FrictionDynamic; }
	void SetFrictionDynamic(float f) { m_FrictionDynamic = f; }
	float FrictionStatic() const { return m_FrictionStatic; }
	void SetFrictionStatic(float f) { m_FrictionStatic = f; }
	float LinDamping() const { return m_LinDamping; }
	void SetLinDamping(float d) { m_LinDamping = d; }
	float AngDamping() const { return m_AngDamping; }
	void SetAngDamping(float d) { m_AngDamping = std::max(0.0f, d); }

	// 重力関係 -----
	void SetGravityMode(GravityMode m) { m_GravityMode = m; }
	GravityMode GravityMode() const	   { return m_GravityMode; }
	void SetGravityScale(float f) { m_GravityScale = f; }
	float GravityScale() const    { return m_GravityScale; }
	void SetCustomGravity(const Vector3& g) { m_CustomGravity = g; }
	Vector3 CustomGravity() const			{ return m_CustomGravity; }

	// Force -----
	void AddForce(const Vector3& f) { m_ForceAccum += f; } // 力を加える
	Vector3 ConsumeForces() { Vector3 f = m_ForceAccum; m_ForceAccum = { 0, 0, 0 }; return f; } // 力を消費
	void ApplyImpulse(const Vector3& P)
	{
		if (!IsDynamic()) return;
		m_Velocity += P * m_InvMass;
	}
	void ApplyImpulseAtPoint(const Vector3& P, const Vector3& worldP)
	{
		if (!IsDynamic()) return;
		m_Velocity += P * m_InvMass;
		const Vector3 r = worldP - WorldCOM();
		m_AngularVelocity += ApplyInvInertiaWorld(Vector3::Cross(r, P));
	}
	void AddTorque(const Vector3& tau) 
	{
		if (!IsDynamic()) return;
		m_TorqueAccum += tau;
	}
	// F : 世界座標の力ベクトル[N]
	// WorldP : その力を加える世界座標上の点[m]
	void AddForceAtPoint(const Vector3& F, const Vector3& worldP)
	{
		if (!IsDynamic()) return;

		// 線形
		m_ForceAccum += F;

		// トルク
		const Vector3 r = worldP - WorldCOM(); // 中心からのベクトル
		const Vector3 tau = Vector3::Cross(r, F); // 距離に依存してトルクの大きさが変わる
		m_TorqueAccum += tau;
	}
	Vector3 ConsumeTorques() { Vector3 t = m_TorqueAccum; m_TorqueAccum = { 0, 0, 0 }; return t; } // トルクの消費
};

#endif