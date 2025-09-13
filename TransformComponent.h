/*
	TransformComponent.h
	20250813  hanaue sho
	Transform�������Ă���R���|�[�l���g
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
    // �f�[�^�{��
	Transform m_Value{}; // ������ Transform �����̂܂ܓ���

    // �e�q�֌W�i�񏊗L�j
    TransformComponent* m_pParent = nullptr;
    std::vector<TransformComponent*> m_Children;

    // �L���b�V��
    mutable Matrix4x4 m_Local{};
    mutable Matrix4x4 m_World{};
    mutable bool m_LocalDirty = true; // �����ύX�����������iCB�X�V��K�w�`���̍œK���Ɏg����j
    mutable bool m_WorldDirty = true; // �����ύX�����������iCB�X�V��K�w�`���̍œK���Ɏg����j

public:
    TransformComponent() = default;
    ~TransformComponent() = default;
	// ----- �A�N�Z�X�i�l���̂��̂ւ̎Q�Ɓj-----
	Transform&       Value()        noexcept { return m_Value; }
	const Transform& Value() const  noexcept { return m_Value; }
    void             SetValue(const Transform& t) noexcept { m_Value = t; MarkLocalDirty();}


	// ----- �֋X�A�N�Z�X -----
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
            Matrix4x4 invParent = m_pParent->WorldMatrix().Inverse(); // �t�s��̎擾
            Vector3 localPos = invParent.TransformPoint(worldPos);
            SetPosition(localPos);
        }
        else
            SetPosition(worldPos); // �e�����Ȃ��̂Ń��[�J�������[���h
    }
    Vector3 WorldPosition() const noexcept
    {
        const Matrix4x4 w = WorldMatrix();
        return Vector3(w.m[3][0], w.m[3][1], w.m[3][2]);
    }

    void SetWorldRotation(const Quaternion& worldQ)
    {
        // �e�̃��[���h�}�g���N�X�̉�]�����𐳋K�����Ď擾
        Matrix4x4 parentMat;
        if (m_pParent) parentMat = m_pParent->WorldMatrix().RotationNormalized();
        else           parentMat.identity();

        // ���͂����K��
        Matrix4x4 rot = worldQ.ToMatrix().RotationNormalized();

        // �s�x�N�g���K��F 
        Matrix4x4 rotParent = parentMat.Transpose(); // �t�s��i�����s��Ȃ̂œ]�u�ɓ������̂œ]�u�j
        Matrix4x4 rotLocal = rot * rotParent; // ���� * �e�̋t�s��

        // �g��]�����h���[�J���ɔ��f�i�ʒu�A�X�P�[���͕ύX���Ȃ��j
        Quaternion localRotation = Quaternion::FromMatrix(rotLocal);
        SetRotation(localRotation);
    }
    Quaternion WorldRotation() const
    {
        return Transform::FromMatrix(WorldMatrix()).rotation;
    }

    bool IsWorldDirty() const noexcept { return m_WorldDirty; }

    // ----- �e�q�֌W�Ǘ� -----
    static bool IsDescendantOf(TransformComponent* node, TransformComponent* maybeAncestor) {
        for (auto* p = node->m_pParent; p; p = p->m_pParent)
            if (p == maybeAncestor) return true;
        return false;
    }

    // �e�̐ݒ� 
    void SetParent(TransformComponent* pParent)
    {
        if (m_pParent == pParent) return; // ���ɐe�ɐݒ肵�Ă���
        if (pParent && IsDescendantOf(pParent, this)) return; // �����̎q��e�ɐݒ肵�Ȃ����߂Ƀ`�F�b�N

        // ���e���玩�����O��
        if (m_pParent)
        {
            auto& v = m_pParent->m_Children;
            v.erase(std::remove(v.begin(), v.end(), this), v.end());
        }

        m_pParent = pParent;

        if (m_pParent) m_pParent->m_Children.push_back(this);

        // �e���ς�����̂Ń��[���h����ς��̂ōČv�Z
        MarkWorldDirtyRecursive();
    }
    // �e�ݒ�i���[���h�ێ��j
    void SetParentKeepWorld(TransformComponent* newParent)
    {
        if (m_pParent == newParent) return; // ���ɐe�ɐݒ肵�Ă���
        if (newParent && IsDescendantOf(newParent, this)) return; // �����̎q��e�ɐݒ肵�Ȃ����߂Ƀ`�F�b�N

        // ���݂̃��[���h���W��ێ�
        Matrix4x4 currentWorld = WorldMatrix();

        // �܂��͒ʏ�� SetParent �Ɠ�������
        if (m_pParent)
        {
            auto& v = m_pParent->m_Children;
            v.erase(std::remove(v.begin(), v.end(), this), v.end());
        }
        m_pParent = newParent;
        if (m_pParent) m_pParent->m_Children.push_back(this);
         
        // �V�������[�J�����W���v�Z���ēK�p
        if (m_pParent)
        {
            Matrix4x4 invParent = m_pParent->WorldMatrix().Inverse();
            Matrix4x4 newLocal = currentWorld * invParent; // Local = World * Parent^-1
            m_Value = Transform::FromMatrix(newLocal);
        }
        else
            m_Value = Transform::FromMatrix(currentWorld);

        MarkLocalDirty(); // ���[���h���X�V�����
    }

    TransformComponent* Parent()                noexcept { return m_pParent; }
    const TransformComponent* Parent() const    noexcept { return m_pParent; }
    const std::vector<TransformComponent*>& Children() const noexcept { return m_Children; }


    // ----- �s��擾�i�x���Čv�Z�j-----
    const Matrix4x4& LocalMatrix()
    {
        if (m_LocalDirty)
        {
            m_Local = m_Value.GetLocalMatrix();

            m_LocalDirty = false;
            m_WorldDirty = true; // ���[�J�����ς�����̂Ń��[���h���X�V
        }
        return m_Local;
    }
    const Matrix4x4& WorldMatrix()
    {
        // ���[�J���X�V
        LocalMatrix(); // ������ WorldDirty �t���O�������Ƃ�����

        if (m_WorldDirty)
        {
            //printf("WorldDirty!");
            if (m_pParent)
                m_World = m_Local * m_pParent->WorldMatrix(); // ���[�J���~�e
            else
                m_World = m_Local; // �e�����Ȃ��̂� World = Local
            m_WorldDirty = false;
        }
        return m_World;
    }
    const Matrix4x4& WorldMatrix() const // �������@�R�C�c�̂����� mutable ���g���Ă��܂��@������
    {
        const_cast<TransformComponent*>(this)->WorldMatrix(); // �����ōČv�Z���Ă���i�H�j
        return m_World;
    }

    // ----- Dirty ���� -----
    // ���[�J���s��̕ύX
    void MarkLocalDirty()
    {
        m_LocalDirty = true;
        MarkWorldDirtyRecursive(); // ���[�J�����ς��̂Ń��[���h���ς���
    }
    // ���[���h�s��ύX����
    void MarkWorldDirtyRecursive()
    {
        m_WorldDirty = true;
        for (auto* ch : m_Children) if (ch) ch->MarkWorldDirtyRecursive(); // �q���S���̍s����Čv�Z
    }


    // ----- ���C�t�T�C�N�� -----
    void OnAdded()  override 
    {
        m_Value.parent = nullptr; // �g��Ȃ��O��Ȃ̂ňꉞ������
        m_LocalDirty = true;
        m_WorldDirty = true;
    }
    void Init()     override {}
    void Uninit()   override 
    {
        // �q�̐e�Q�Ƃ��O���i���|���j-----
        for (auto* ch : m_Children) if (ch) ch->m_pParent = nullptr;
        m_Children.clear();
        SetParent(nullptr);
    }

    // Transform �͒ʏ� Update/Draw �ŏ����s�v�B�K�w������Ȃ炱���Ń��[���h�X�V�Ȃ�
    void FixedUpdate(float dt)  override {}
    void Update(float dt)       override {}
    void Draw()                 override {}

    
};
static_assert(!std::is_abstract<TransformComponent>::value, "TransformComponent is abstract");
static_assert(std::is_default_constructible<TransformComponent>::value, "TransformComponent is NOT default-constructible");

#endif