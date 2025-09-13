/*
	TransformComponent.h
	20250813  hanaue sho
	Transformを持っているコンポーネント
*/
#ifndef TRANSFORMCOMPONENT_H_
#define TRANSFORMCOMPONENT_H_
#include <vector>
#include <algorithm>
#include "Component.h"
#include "Transform.h"
#include "Matrix4x4.h"

#include <type_traits>
class TransformComponent : public Component
{
protected:
    // データ本体
	Transform m_Value{}; // 既存の Transform をそのまま内包

    // 親子関係（非所有）
    TransformComponent* m_pParent = nullptr;
    std::vector<TransformComponent*> m_Children;

    // キャッシュ
    mutable Matrix4x4 m_Local{};
    mutable Matrix4x4 m_World{};
    mutable bool m_LocalDirty = true; // 何か変更があったか（CB更新や階層伝搬の最適化に使える）
    mutable bool m_WorldDirty = true; // 何か変更があったか（CB更新や階層伝搬の最適化に使える）

public:
    TransformComponent() = default;
    ~TransformComponent() = default;
	// ----- アクセス（値そのものへの参照）-----
	Transform&       Value()        noexcept { return m_Value; }
	const Transform& Value() const  noexcept { return m_Value; }
    void             SetValue(const Transform& t) noexcept { m_Value = t; MarkLocalDirty();}


	// ----- 便宜アクセス -----
    void     SetPosition(const Vector3& p)          noexcept { m_Value.position = p; MarkLocalDirty(); }
    Vector3  Position()                     const   noexcept { return m_Value.position; }

    void     SetRotation(const Quaternion& q)       noexcept { m_Value.rotation = q; MarkLocalDirty(); }
    Quaternion Rotation()                   const   noexcept { return m_Value.rotation; }

    void     SetEulerAngles(const Vector3& e)       noexcept { m_Value.rotation = Quaternion::FromEulerAngles(e); MarkLocalDirty(); }
    Vector3  EulerAngles()                  const   noexcept { return m_Value.rotation.ToEulerAngles(); }

    void     SetScale(const Vector3& s)             noexcept { m_Value.scale = s; MarkLocalDirty(); }
    Vector3  Scale()                        const   noexcept { return m_Value.scale; }

    Vector3  Right()    const   noexcept { return m_Value.GetRight(); }
    Vector3  Up()       const   noexcept { return m_Value.GetUp(); }
    Vector3  Forward()  const   noexcept { return m_Value.GetForward(); }

    void SetWorldPosition(const Vector3& worldPos)
    {
        if (m_pParent)
        {
            Matrix4x4 invParent = m_pParent->WorldMatrix().Inverse(); // 逆行列の取得
            Vector3 localPos = invParent.TransformPoint(worldPos);
            SetPosition(localPos);
        }
        else
            SetPosition(worldPos); // 親がいないのでローカル＝ワールド
    }
    Vector3 WorldPosition() const noexcept
    {
        const Matrix4x4 w = WorldMatrix();
        return Vector3(w.m[3][0], w.m[3][1], w.m[3][2]);
    }

    void SetWorldRotation(const Quaternion& worldQ)
    {
        // 親のワールドマトリクスの回転成分を正規化して取得
        Matrix4x4 parentMat;
        if (m_pParent) parentMat = m_pParent->WorldMatrix().RotationNormalized();
        else           parentMat.identity();

        // 入力も正規化
        Matrix4x4 rot = worldQ.ToMatrix().RotationNormalized();

        // 行ベクトル規約： 
        Matrix4x4 rotParent = parentMat.Transpose(); // 逆行列（直交行列なので転置に等しいので転置）
        Matrix4x4 rotLocal = rot * rotParent; // 自分 * 親の逆行列

        // “回転だけ”ローカルに反映（位置、スケールは変更しない）
        Quaternion localRotation = Quaternion::FromMatrix(rotLocal);
        SetRotation(localRotation);
    }
    Quaternion WorldRotation() const
    {
        return Transform::FromMatrix(WorldMatrix()).rotation;
    }

    bool IsWorldDirty() const noexcept { return m_WorldDirty; }

    // ----- 親子関係管理 -----
    static bool IsDescendantOf(TransformComponent* node, TransformComponent* maybeAncestor) {
        for (auto* p = node->m_pParent; p; p = p->m_pParent)
            if (p == maybeAncestor) return true;
        return false;
    }

    // 親の設定 
    void SetParent(TransformComponent* pParent)
    {
        if (m_pParent == pParent) return; // 既に親に設定している
        if (pParent && IsDescendantOf(pParent, this)) return; // 自分の子を親に設定しないためにチェック

        // 旧親から自分を外す
        if (m_pParent)
        {
            auto& v = m_pParent->m_Children;
            v.erase(std::remove(v.begin(), v.end(), this), v.end());
        }

        m_pParent = pParent;

        if (m_pParent) m_pParent->m_Children.push_back(this);

        // 親が変わったのでワールド基準が変わるので再計算
        MarkWorldDirtyRecursive();
    }
    // 親設定（ワールド維持）
    void SetParentKeepWorld(TransformComponent* newParent)
    {
        if (m_pParent == newParent) return; // 既に親に設定している
        if (newParent && IsDescendantOf(newParent, this)) return; // 自分の子を親に設定しないためにチェック

        // 現在のワールド座標を保持
        Matrix4x4 currentWorld = WorldMatrix();

        // まずは通常の SetParent と同じ処理
        if (m_pParent)
        {
            auto& v = m_pParent->m_Children;
            v.erase(std::remove(v.begin(), v.end(), this), v.end());
        }
        m_pParent = newParent;
        if (m_pParent) m_pParent->m_Children.push_back(this);
         
        // 新しいローカル座標を計算して適用
        if (m_pParent)
        {
            Matrix4x4 invParent = m_pParent->WorldMatrix().Inverse();
            Matrix4x4 newLocal = currentWorld * invParent; // Local = World * Parent^-1
            m_Value = Transform::FromMatrix(newLocal);
        }
        else
            m_Value = Transform::FromMatrix(currentWorld);

        MarkLocalDirty(); // ワールドも更新される
    }

    TransformComponent* Parent()                noexcept { return m_pParent; }
    const TransformComponent* Parent() const    noexcept { return m_pParent; }
    const std::vector<TransformComponent*>& Children() const noexcept { return m_Children; }


    // ----- 行列取得（遅延再計算）-----
    const Matrix4x4& LocalMatrix()
    {
        if (m_LocalDirty)
        {
            m_Local = m_Value.GetLocalMatrix();

            m_LocalDirty = false;
            m_WorldDirty = true; // ローカルが変わったのでワールドも更新
        }
        return m_Local;
    }
    const Matrix4x4& WorldMatrix()
    {
        // ローカル更新
        LocalMatrix(); // ここで WorldDirty フラグが立つことがある

        if (m_WorldDirty)
        {
            //printf("WorldDirty!");
            if (m_pParent)
                m_World = m_Local * m_pParent->WorldMatrix(); // ローカル×親
            else
                m_World = m_Local; // 親がいないので World = Local
            m_WorldDirty = false;
        }
        return m_World;
    }
    const Matrix4x4& WorldMatrix() const // ※※※　コイツのせいで mutable を使っています　※※※
    {
        const_cast<TransformComponent*>(this)->WorldMatrix(); // ここで再計算している（？）
        return m_World;
    }

    // ----- Dirty 操作 -----
    // ローカル行列の変更
    void MarkLocalDirty()
    {
        m_LocalDirty = true;
        MarkWorldDirtyRecursive(); // ローカルが変わるのでワールドも変える
    }
    // ワールド行列変更命令
    void MarkWorldDirtyRecursive()
    {
        m_WorldDirty = true;
        for (auto* ch : m_Children) if (ch) ch->MarkWorldDirtyRecursive(); // 子供全員の行列を再計算
    }


    // ----- ライフサイクル -----
    void OnAdded()  override 
    {
        m_Value.parent = nullptr; // 使わない前提なので一応無効化
        m_LocalDirty = true;
        m_WorldDirty = true;
    }
    void Init()     override {}
    void Uninit()   override 
    {
        // 子の親参照を外す（お掃除）-----
        for (auto* ch : m_Children) if (ch) ch->m_pParent = nullptr;
        m_Children.clear();
        SetParent(nullptr);
    }

    // Transform は通常 Update/Draw で処理不要。階層化するならここでワールド更新など
    void FixedUpdate(float dt)  override {}
    void Update(float dt)       override {}
    void Draw()                 override {}

    
};
static_assert(!std::is_abstract<TransformComponent>::value, "TransformComponent is abstract");
static_assert(std::is_default_constructible<TransformComponent>::value, "TransformComponent is NOT default-constructible");

#endif