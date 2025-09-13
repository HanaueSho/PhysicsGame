/*
	Transform.h
	20250521  hanaue sho
*/
#ifndef TRANSFORM_H_
#define TRANSFORM_H_
#include <math.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"

struct Transform
{
public:
	// �ϐ�
	Vector3 position = {0.0f, 0.0f, 0.0f};
	Quaternion rotation = Quaternion::Identity(); // �N�H�[�^�j�I��
	Vector3 scale = { 1.0f, 1.0f, 1.0f };
	Transform* parent = nullptr; // �e�g�����X�t�H�[���i���ݎg���Ă���܂���j

    // �R���X�g���N�^
	Transform() {}

	// �֐�
	Matrix4x4 GetLocalMatrix() const // �i���݂̃v���W�F�N�g�ł͎g���܂���j
	{
		Matrix4x4 Scale = Matrix4x4::CreateScale(scale.x, scale.y, scale.z);
		Matrix4x4 Rotation = rotation.ToMatrix(); // ��]�s��
		Matrix4x4 Translation = Matrix4x4::CreateTranslation(position.x, position.y, position.z);
		return Scale * Rotation * Translation; // S��R��T �̏���
	}
	Matrix4x4 GetWorldMatrix() const // �i���݂̃v���W�F�N�g�ł͎g���܂���j
	{
		if (parent)
			return GetLocalMatrix() * parent->GetWorldMatrix(); // �q�̃��[�J�����e�̃��[���h
		else
			return GetLocalMatrix();
	}

	Vector3 GetRight() const	{ return rotation.Rotate(Vector3(1, 0, 0)); }
	Vector3 GetUp() const		{ return rotation.Rotate(Vector3(0, 1, 0)); }
	Vector3 GetForward() const	{ return rotation.Rotate(Vector3(0, 0, 1)); }
	
	// ��]������֐��i���[�J������]�j
	void Rotate(const Vector3& deltaEuler)
	{
		Quaternion delta = Quaternion::FromEulerAngles(deltaEuler);
		rotation = (rotation * delta).normalized();
	}

	// �C�ӂ̎��ŉ�]
	void RotateAxis(const Vector3& worldAxis, float angle)
	{
		Quaternion delta = Quaternion::FromAxisAngle(worldAxis, angle);
		rotation = (delta * rotation).normalized();
	}

	// �^�[�Q�b�g�̕�����������
	void LookAt(const Vector3& target, const Vector3& up = Vector3(0, 1, 0))
	{
		rotation = Quaternion::LookAt(position, target, up);
	}

	// Matrix����Transform�𐶐�
	static Transform FromMatrix(const Matrix4x4& mat, float eps = 1e-6f) noexcept
	{
		Transform out;
		out.parent = nullptr;

		// �P�jTranslation
		out.position = Vector3(mat.m[3][0], mat.m[3][1], mat.m[3][2]);

        // 2) Scale�F3x3 �̊e�u�s�v�̒����i�s�x�N�g�����V�j
        Vector3 row0 = Vector3(mat.m[0][0], mat.m[0][1], mat.m[0][2]);
        Vector3 row1 = Vector3(mat.m[1][0], mat.m[1][1], mat.m[1][2]);
        Vector3 row2 = Vector3(mat.m[2][0], mat.m[2][1], mat.m[2][2]);

        float sx = row0.length();
        float sy = row1.length();
        float sz = row2.length();

        if (sx < eps) sx = 1.0f;
        if (sy < eps) sy = 1.0f;
        if (sz < eps) sz = 1.0f;

        out.scale = Vector3(sx, sy, sz);

        // 3) Rotation�F�X�P�[���������� 3x3 �𐳋K���i�s�𐳋K���j
        float r00 = mat.m[0][0] / sx, r01 = mat.m[0][1] / sx, r02 = mat.m[0][2] / sx;
        float r10 = mat.m[1][0] / sy, r11 = mat.m[1][1] / sy, r12 = mat.m[1][2] / sy;
        float r20 = mat.m[2][0] / sz, r21 = mat.m[2][1] / sz, r22 = mat.m[2][2] / sz;

        // ���]�X�P�[���i�s��̔��ˁj�΍�Fdet<0 �̏ꍇ�͈�̎��𔽓]
        float det = r00 * (r11 * r22 - r12 * r21) - r01 * (r10 * r22 - r12 * r20) + r02 * (r10 * r21 - r11 * r20);
        if (det < 0.0f) {
            // ����Ƃ��� Z �𔽓]�i�ǂ�ł��ǂ�����ѐ����厖�j
            sz = -sz;
            out.scale.z = sz;
            r20 = -r20; r21 = -r21; r22 = -r22;
            det = -det;
        }

        // 3x3 ��]�s�� �� �N�H�[�^�j�I���i�s��� row-major �̉�]�j
        // �W���̎�@�i���l����Łj
        float trace = r00 + r11 + r22;
        Quaternion q;
        if (trace > 0.0f) {
            float s = sqrt(trace + 1.0f) * 2.0f; // s=4*qw
            q.w = 0.25f * s;
            q.x = (r21 - r12) / s;
            q.y = (r02 - r20) / s;
            q.z = (r10 - r01) / s;
        }
        else if (r00 > r11 && r00 > r22) {
            float s = sqrt(1.0f + r00 - r11 - r22) * 2.0f; // s=4*qx
            q.w = (r21 - r12) / s;
            q.x = 0.25f * s;
            q.y = (r01 + r10) / s;
            q.z = (r02 + r20) / s;
        }
        else if (r11 > r22) {
            float s = sqrt(1.0f + r11 - r00 - r22) * 2.0f; // s=4*qy
            q.w = (r02 - r20) / s;
            q.x = (r01 + r10) / s;
            q.y = 0.25f * s;
            q.z = (r12 + r21) / s;
        }
        else {
            float s = sqrt(1.0f + r22 - r00 - r11) * 2.0f; // s=4*qz
            q.w = (r10 - r01) / s;
            q.x = (r02 + r20) / s;
            q.y = (r12 + r21) / s;
            q.z = 0.25f * s;
        }
        q = q.normalized(); // �O�̂��ߐ��K��

        out.rotation = q;
        return out;
	}
		
};

#endif TRANSFORM_H_