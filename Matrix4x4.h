/*
	Matrix4x4.h
	20250806  hanaue sho
	�s�x�N�g�����V�i�S�s�ڂɕ��s�ړ������j
*/
#ifndef MATRIX4X4_H_
#define MATRIX4X4_H_
#include <cstring>
#include <math.h>
#include "Vector3.h"

class Matrix4x4
{
public:
	float m[4][4]; // �s��{��

	Matrix4x4() { identity(); }

	// �P�ʍs��ɏ�����
	void identity()
	{
		memset(m, 0, sizeof(m)); // ������
		m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
	}

	// ����3x3�݂̂��[����
	void Zero3x3()
	{
		memset(m, 0, sizeof(m)); // ������
		m[3][3] = 1.0f; // [3][3]�݂̂P�C����ȊO�͂O��
	}

	// �s�񓯎m�̊|���Z
	Matrix4x4 operator *(const Matrix4x4& rhs) const
	{
		Matrix4x4 result;
		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
			{
				result.m[row][col] = 0.0f;
				for (int k = 0; k < 4; k++)
				{
					result.m[row][col] += m[row][k] * rhs.m[k][col];
				}

			}
		}
		return result;
	}

	// �s��� float �̊|���Z�i�e�v�f�� float ���|����j
	Matrix4x4 operator *(float f) const
	{
		Matrix4x4 result;
		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
			{
				result.m[row][col] = m[row][col] * f; // �e������ float ���|����
			}
		}
		return result;
	}

	// ���s�ړ��s��
	static Matrix4x4 CreateTranslation(float x, float y, float z)
	{
		Matrix4x4 mat;
		mat.identity();
		mat.m[3][0] = x;
		mat.m[3][1] = y;
		mat.m[3][2] = z;
		return mat;
	}

	// �X�P�[���s��
	static Matrix4x4 CreateScale(float sx, float sy, float sz)
	{
		Matrix4x4 mat;
		mat.identity();
		mat.m[0][0] = sx;
		mat.m[1][1] = sy;
		mat.m[2][2] = sz;
		return mat;
	}

	// ��]�s�� �iYaw��Pitch��Roll�j�S�ă��W�A���i������W�n�j
	static Matrix4x4 CreateRotationYawPitchRoll(float yaw, float pitch, float roll)
	{
		float cy = cos(yaw)  , sy = sin(yaw);
		float cp = cos(pitch), sp = sin(pitch);
		float cr = cos(roll) , sr = sin(roll);

		Matrix4x4 mat;
		mat.identity();

		mat.m[0][0] = cy * cr + sy * sp * sr;
		mat.m[0][1] = sr * cp;
		mat.m[0][2] = -sy * cr + cy * sp * sr;
		mat.m[0][3] = 0.0f;

		mat.m[1][0] = -cy * sr + sy * sp * cr;
		mat.m[1][1] = cr * cp;
		mat.m[1][2] = sr * sy + cy * sp * cr;
		mat.m[1][3] = 0.0f;

		mat.m[2][0] = sy * cp;
		mat.m[2][1] = -sp;
		mat.m[2][2] = cy * cp;
		mat.m[2][3] = 0.0f;

		mat.m[3][0] = mat.m[3][1] = mat.m[3][2] = 0.0f;
		mat.m[3][3] = 1.0f;

		return mat;
	}

	// ��]�s��i���A�p�x�����]�s������j
	static Matrix4x4 CreateRotationAxis(const Vector3& axis, float angle)
	{
		Vector3 a = axis.normalized();
		float c = cosf(angle);
		float s = sinf(angle);
		float t = 1.0f - c;

		Matrix4x4 mat;
		mat.identity();

		mat.m[0][0] = t * a.x * a.x + c;
		mat.m[0][1] = t * a.x * a.y + s * a.z;
		mat.m[0][2] = t * a.x * a.z - s * a.y;

		mat.m[1][0] = t * a.x * a.y - s * a.z;
		mat.m[1][1] = t * a.y * a.y + c;
		mat.m[1][2] = t * a.y * a.z + s * a.x;

		mat.m[2][0] = t * a.x * a.z + s * a.y;
		mat.m[2][1] = t * a.y * a.z - s * a.x;
		mat.m[2][2] = t * a.z * a.z + c;

		return mat;
	}

	// Transform ����Matrix �𐶐�
	static Matrix4x4 CreateTRS(const Vector3& pos, const Matrix4x4& rot3x3, const Vector3& s)
	{
		Matrix4x4 M = rot3x3;
		// �s�X�P�[�� (S * R)
		M.m[0][0] *= s.x; M.m[0][1] *= s.x; M.m[0][2] *= s.x;
		M.m[1][0] *= s.y; M.m[1][1] *= s.y; M.m[1][2] *= s.y;
		M.m[2][0] *= s.z; M.m[2][1] *= s.z; M.m[2][2] *= s.z;
		// ���s�ړ� (S * R * T)
		M.m[3][0] = pos.x; M.m[3][1] = pos.y; M.m[3][2] = pos.z;
		M.m[0][3] = M.m[1][3] = M.m[2][3] = 0.0f; M.m[3][3] = 1.0f;
		return M;
	}

	// �]�u����
	Matrix4x4 Transpose() const 
	{
		Matrix4x4 t;
		for (int r = 0; r < 4; ++r)
			for (int c = 0; c < 4; ++c)
				t.m[r][c] = m[c][r];
		return t;
	}

	// �t�s��
	Matrix4x4 Inverse() const
	{
		Matrix4x4 inv;
		inv.identity();

		// 3x3 �̋t�s����v�Z�i��]�{�X�P�[�������j
		float det =
			m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
			m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
			m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

		if (fabs(det) < 1e-8f) {
			// �t�s�񂪑��݂��Ȃ�
			return inv; // �P�ʍs���Ԃ��ȂǓK�X�G���[����
		}

		// �����Ŋ���
		float invDet = 1.0f / det;

		inv.m[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
		inv.m[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * invDet;
		inv.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;

		inv.m[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * invDet;
		inv.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
		inv.m[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * invDet;

		inv.m[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
		inv.m[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * invDet;
		inv.m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;

		// ���s�ړ��̋t���v�Z
		inv.m[3][0] = -(m[3][0] * inv.m[0][0] + m[3][1] * inv.m[1][0] + m[3][2] * inv.m[2][0]);
		inv.m[3][1] = -(m[3][0] * inv.m[0][1] + m[3][1] * inv.m[1][1] + m[3][2] * inv.m[2][1]);
		inv.m[3][2] = -(m[3][0] * inv.m[0][2] + m[3][1] * inv.m[1][2] + m[3][2] * inv.m[2][2]);

		return inv;
	}

	// �s�񂩂�I�C���[�p�֕ϊ�
	Vector3 ToEulerAngles() const
	{
		Vector3 euler;

		// pitch(X����])
		if (fabs(m[2][1]) < 0.9999f)
		{
			euler.x = asinf(m[2][1]); // pitch
			euler.y = atan2f(-m[2][0], m[2][2]); // yaw
			euler.z = atan2f(-m[0][1], m[1][1]); // roll
		}
		else
		{
			// �W���o�����b�N�im[2][1] �� �}�P�j�̏���
			euler.x = (m[2][1] > 0.0f) ? PI / 2 : -PI / 2;
			euler.y = atan2f(m[0][2], m[0][0]);
			euler.z = 0.0f;
		}
		return euler;
	}

	// �x�N�g���ɑ΂��ĉ�]�s���K�p
	Vector3 TransformNormal(const Vector3& vect) const
	{
		// [�s][��]
		return Vector3(
			vect.x * m[0][0] + vect.y * m[1][0] + vect.z * m[2][0],
			vect.x * m[0][1] + vect.y * m[1][1] + vect.z * m[2][1],
			vect.x * m[0][2] + vect.y * m[1][2] + vect.z * m[2][2]
		);
	}

	// �x�N�g���ɑ΂��ĉ�]�{���s�ړ���K�p�i���[���h���W�ϊ��j
	Vector3 TransformPoint(const Vector3& vect) const
	{
		return Vector3(
			vect.x * m[0][0] + vect.y * m[1][0] + vect.z * m[2][0] + m[3][0],
			vect.x * m[0][1] + vect.y * m[1][1] + vect.z * m[2][1] + m[3][1],
			vect.x * m[0][2] + vect.y * m[1][2] + vect.z * m[2][2] + m[3][2]
		);
	}

	// �s�񂩂��]���݂̂����o��
	Matrix4x4 RotationNormalized() const
	{
		// �e�s���������o��
		Vector3 r0(m[0][0], m[0][1], m[0][2]);
		Vector3 r1(m[1][0], m[1][1], m[1][2]);
		Vector3 r2(m[2][0], m[2][1], m[2][2]);

		// �e�s�̃X�P�[�������o��
		float sx = r0.length(); if (sx < 1e-8f) { r0 = Vector3(1, 0, 0); sx = 1; } else {r0 *= (1.0f / sx); }
		float sy = r1.length(); if (sy < 1e-8f) { r1 = Vector3(0, 1, 0); sy = 1; } else {r1 *= (1.0f / sy); }
		float sz = r2.length(); if (sz < 1e-8f) { r2 = Vector3(0, 0, 1); sz = 1; } else {r2 *= (1.0f / sz); }

		// ���]���o
		if (Vector3::Dot(Vector3::Cross(r0, r1), r2) < 0.0f) r2 *= -1.0f;

		// ���K�����ꂽ��]����Ԃ�
		Matrix4x4 R; R.identity();
		R.m[0][0] = r0.x; R.m[0][1] = r0.y; R.m[0][2] = r0.z;
		R.m[1][0] = r1.x; R.m[1][1] = r1.y; R.m[1][2] = r1.z;
		R.m[2][0] = r2.x; R.m[2][1] = r2.y; R.m[2][2] = r2.z;
		return R;
	}

};

#endif