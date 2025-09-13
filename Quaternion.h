/*
	Quaternion.h
	20250806  hanaue sho
*/
#ifndef QUATERNION_H_
#define QUATERNION_H_
#include <math.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "MathCommon.h"

class Quaternion
{
public:
	float x, y, z, w; // �{�̗v�f

	// �R���X�g���N�^
	Quaternion() : x(0), y(0), z(0), w(1) { }
	Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	// ���Ɗp�x����N�H�[�^�j�I���𐶐�
	static Quaternion FromAxisAngle(const Vector3& axis, float angle)
	{
		Vector3 normal = axis.normalized(); // ���𐳋K��
		float sinHalfAngle = sinf(angle * 0.5f);
		float cosHalfAngle = cosf(angle * 0.5f);

		return Quaternion(
			normal.x * sinHalfAngle,
			normal.y * sinHalfAngle,
			normal.z * sinHalfAngle,
			cosHalfAngle
		);
	}

	// �I�C���[�e���琶���iYaw��Pitch��Roll�j
	// ���W���o�����b�N�͉���ł��Ȃ��̂Œ��Ӂ�
	static Quaternion FromEulerAngles(const Vector3& euler)
	{
		float cy = cosf(euler.y * 0.5f), sy = sinf(euler.y * 0.5f); // yaw
		float cp = cosf(euler.x * 0.5f), sp = sinf(euler.x * 0.5f); // pitch
		float cr = cosf(euler.z * 0.5f), sr = sinf(euler.z * 0.5f); // roll
		
		return Quaternion(
			cp * sy * sr + cy * cr * sp,
			cp * cr * sy - cy * sp * sr,
			cp * cy * sr - cr * sp * sy,
			cp * cy * cr + sp * sy * sr
		);
	}

	// �N�H�[�^�j�I�����m�̐ρi��]�����j
	Quaternion operator *(const Quaternion& rhs) const
	{
		return Quaternion(
			w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
			w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
			w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
			w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
		);
	}

	// ���K��
	Quaternion normalized() const
	{
		float magSq = x * x + y * y + z * z + w * w;
		if (magSq > 0.0f)
		{
			float invMag = 1.0f / sqrtf(magSq);
			return Quaternion(x * invMag, y * invMag, z * invMag, w * invMag);
		}
		return Identity(); // ���s���͒P�ʃN�H�[�^�j�I��
	}

	// �^����ꂽ�x�N�g�����N�H�[�^�j�I���ŉ񂷁i�K�p������j
	Vector3 Rotate(const Vector3& vect) const
	{
		Quaternion qv(vect.x, vect.y, vect.z, 0.0f);
		Quaternion qConj(-x, -y, -z, w);
		Quaternion result = (*this) * qv * qConj;

		return Vector3(result.x, result.y, result.z);
	}

	// �P�ʃN�H�[�^�j�I����Ԃ�
	static Quaternion Identity() { return Quaternion(0, 0, 0, 1); }

	// ��]�s��ɕϊ�
	Matrix4x4 ToMatrix() const
	{
		float xx = x * x, yy = y * y, zz = z * z;
		float xy = x * y, xz = x * z, yz = y * z;
		float wx = w * x, wy = w * y, wz = w * z;

		Matrix4x4 mat;
		mat.identity();

		mat.m[0][0] = 1.0f - 2.0f * (yy + zz);
		mat.m[0][1] = 2.0f * (xy + wz);
		mat.m[0][2] = 2.0f * (xz - wy);

		mat.m[1][0] = 2.0f * (xy - wz);
		mat.m[1][1] = 1.0f - 2.0f * (xx + zz);
		mat.m[1][2] = 2.0f * (yz + wx);

		mat.m[2][0] = 2.0f * (xz + wy);
		mat.m[2][1] = 2.0f * (yz - wx);
		mat.m[2][2] = 1.0f - 2.0f * (xx + yy);

		return mat;
	}

	// �N�H�[�^�j�I������I�C���[�p���擾�iyaw��pitch��roll�j
	Vector3 ToEulerAngles() const
	{
		Vector3 euler;

		//pitch�i������]�j
		float sinp = 2.0f * (w * x + y * z);
		float cosp = 1.0f - 2.0f * (x * x + y * y);
		euler.x = atan2f(sinp, cosp);

		// Yaw�i������]�j
		float siny = 2.0f * (w * y - z * x);
		if (fabs(siny) >= 1.0f)
			euler.y = copysignf(PI/2.0f, siny); // �}90�x�i�W���o�����b�N�j
		else
			euler.y = asinf(siny);

		// Roll�i������]�j
		float sinr = 2.0f * (w * z + x * y);
		float cosr = 1.0f - 2.0f * (y * y + z * z);
		euler.z = atan2f(sinr, cosr);

		return euler;
	}

	// ���Ɗp�x�����o��
	void ToAxisAngle(Vector3& outAxis, float& outAngle) const
	{
		Quaternion q = this->normalized();

		// �p�x�̎擾
		float angle = 2.0f * acosf(q.w);
		outAngle = angle;

		// ���̎擾
		float sinHalfAngle = sqrtf(1.0f - q.w * q.w);
		if (sinHalfAngle < 0.0001f) // �p�x�O�Ȃ̂Ŏ��͔C�Ӂi�P�ʃx�N�g����Ԃ��j
			outAxis = Vector3(1.0f, 0.0f, 0.0f);
		else
			outAxis = Vector3(q.x, q.y, q.z) / sinHalfAngle;
	}

	// from to �ō����x�N�g���֌����N�H�[�^�j�I���𐶐�
	static Quaternion LookAt(const Vector3& from, const Vector3& to, const Vector3& up = Vector3(0, 1, 0))
	{
		Vector3 forward = (to - from).normalized();
		Vector3 right = Vector3::Cross(up, forward).normalized();
		Vector3 newUp = Vector3::Cross(forward, right);

		// ��]�s������
		Matrix4x4 rot;
		rot.identity();

		rot.m[0][0] = right.x;
		rot.m[1][0] = right.y;
		rot.m[2][0] = right.z;

		rot.m[0][1] = newUp.x;
		rot.m[1][1] = newUp.y;
		rot.m[2][1] = newUp.z;

		rot.m[0][2] = forward.x;
		rot.m[1][2] = forward.y;
		rot.m[2][2] = forward.z;

		// �s�񂩂�N�H�[�^�j�I�������
		return Quaternion::FromMatrix(rot);
	}

	// ��]�s�񂩂�N�H�[�^�j�I���𐶐��i�����K�����s��ł���K�v����j
	static Quaternion FromMatrix(const Matrix4x4& rot)
	{
		Quaternion q;
		float trace = rot.m[0][0] + rot.m[1][1] + rot.m[2][2];

		if (trace > 0.0f)
		{
			float s = sqrtf(trace + 1.0f) * 2.0f;
			q.w = 0.25f * s;
			q.x = (rot.m[2][1] - rot.m[1][2]) / s;
			q.y = (rot.m[0][2] - rot.m[2][0]) / s;
			q.z = (rot.m[1][0] - rot.m[0][1]) / s;
		}
		else
		{
			if (rot.m[0][0] > rot.m[1][1] && rot.m[0][0] > rot.m[2][2])
			{
				float s = sqrtf(1.0f + rot.m[0][0] - rot.m[1][1] - rot.m[2][2]) * 2.0f;
				q.w = (rot.m[2][1] - rot.m[1][2]) / s;
				q.x = 0.25f * s;
				q.y = (rot.m[0][1] + rot.m[1][0]) / s;
				q.z = (rot.m[0][2] + rot.m[2][0]) / s;
			}
			else if (rot.m[1][1] > rot.m[2][2])
			{
				float s = sqrtf(1.0f + rot.m[1][1] - rot.m[0][0] - rot.m[2][2]) * 2.0f;
				q.w = (rot.m[0][2] - rot.m[2][0]) / s;
				q.x = (rot.m[0][1] + rot.m[1][0]) / s;
				q.y = 0.25f * s;
				q.z = (rot.m[1][2] + rot.m[2][1]) / s;
			}
			else
			{
				float s = sqrtf(1.0f + rot.m[2][2] - rot.m[0][0] - rot.m[1][1]) * 2.0f;
				q.w = (rot.m[1][0] - rot.m[0][1]) / s;
				q.x = (rot.m[0][2] + rot.m[2][0]) / s;
				q.y = (rot.m[1][2] + rot.m[2][1]) / s;
				q.z = 0.25f * s;
			}
		}
		return q;
	}

	// from ���� to �։񂷂��߂̃N�H�[�^�j�I���𐶐��i�����j
	static Quaternion FromToRotation(const Vector3& from, const Vector3& to)
	{
		Vector3 v0 = from.normalized();
		Vector3 v1 = to.normalized();

		float dot = Vector3::Dot(v0, v1); //����

		if (dot > 0.9999f)// �قړ�������
		{
			return Quaternion::Identity(); // ��]����K�v�Ȃ�
		}
		else if (dot < -0.9999f) // �قڋt�����i�P�W�O�x��]�j
		{
			// �C�ӂ̐����x�N�g������]���Ɂi���萫�̂��߁j
			Vector3 orth = Vector3(1.0f, 0.0f, 0.0f);
			if (fabs(v0.x) > 0.9f) // ���Ǝ��Ă�Ȃ炙������
				orth = Vector3(0.0f, 1.0f, 0.0f);
			Vector3 axis = Vector3::Cross(v0, orth).normalized();
			return Quaternion::FromAxisAngle(axis, PI);

		}
		else
		{
			Vector3 axis = Vector3::Cross(v0, v1).normalized();
			float angle = acosf(dot);
			return Quaternion::FromAxisAngle(axis, angle);
		}
	}

	// �N�H�[�^�j�I���̋��ʐ��`���
	static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t)
	{
		Quaternion a = q1.normalized();
		Quaternion b = q2.normalized();

		// ���ςŉ�]�̊p�x���v�Z
		float  dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

		// �⊮�o�H�������ɂȂ�̂�h��
		if (dot < 0.0f)
		{
			b = Quaternion(-b.x, -b.y, -b.z, -b.w);
			dot = -dot;
		}

		const float DOT_THRESHOULD = 0.9995f;
		if (dot > DOT_THRESHOULD)
		{
			// ���`��ԁi�����������̂Ő��x�I�ɖ��Ȃ��j
			Quaternion result = Quaternion(
				a.x + t * (b.x - a.x),
				a.y + t * (b.y - a.y),
				a.z + t * (b.z - a.z),
				a.w + t * (b.w - a.w)
			);
			return result.normalized();
		}

		// ���ʐ��`���
		float theta = acosf(dot);		// �� = angle between a and b
		float sinTheta = sinf(theta);	// sin(��)
		float tTheta = theta * t;		// t��

		float scaleA = sinf(theta - tTheta) / sinTheta;
		float scaleB = sinf(tTheta) / sinTheta;

		return Quaternion(
			a.x * scaleA + b.x * scaleB,
			a.y * scaleA + b.y * scaleB,
			a.z * scaleA + b.z * scaleB,
			a.w * scaleA + b.w * scaleB
		).normalized();
	}

};

#endif