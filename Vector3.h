/*
	Vector3 �̃N���X
	20250513 hanaue sho
*/
#ifndef VECTOR3_H_
#define VECTOR3_H_
#include <math.h>
#include <cassert>
#include <string>
#include "MathCommon.h"

class Vector3
{
public:
	float x = 0, y = 0, z = 0;
	Vector3() {}
	Vector3(const Vector3& a) : x(a.x), y(a.y), z(a.z) {}
	Vector3(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {}

// �W���I�ȃI�u�W�F�N�g�̕ێ�
	// ����iC����̊��K�ɏ]���l�ւ̎Q�Ƃ�Ԃ��j
	Vector3& operator = (const Vector3& a)
	{ x = a.x; y = a.y; z = a.z; return *this; }

	// �������`�F�b�N
	bool operator == (const Vector3& a) const
	{ return x == a.x && y == a.y && z == a.z; }

	bool operator != (const Vector3& a) const
	{ return x != a.x || y != a.y || z != a.z; }

// �x�N�g������
	// �x�N�g�����O�ɂ���
	void zero() { x = y = z = 0.0f; }

	Vector3 operator +() const { return *this; }
	// �P�����̃}�C�i�X�́A���]�����x�N�g����Ԃ�
	Vector3 operator -() const { return Vector3(-x, -y, -z); }

	// �񍀎��́{�Ɓ|�̓x�N�g�������Z���A���Z����
	Vector3 operator +(const Vector3& a) const 
	{ return Vector3(x + a.x, y + a.y, z + a.z); }
	
	Vector3 operator -(const Vector3& a) const
	{ return Vector3(x - a.x, y - a.y, z - a.z); }

	// �X�J���[�ɂ���Z�Ə��Z
	Vector3 operator *(float a) const
	{ return Vector3(x * a, y * a, z * a); }

	// �x�N�g���̊e�v�f�̏�Z
	Vector3 operator *(const Vector3& a) const
	{
		return Vector3(x * a.x, y * a.y, z * a.z);
	}

	Vector3 operator /(float a) const
	{
		if (a == 0.0f) return Vector3(0, 0, 0);// ���[�����Z�`�F�b�N
		float oneOverA = 1.0f / a; 
		return Vector3(x * oneOverA, y * oneOverA, z * oneOverA); 
	}

	// �v�f�̎��o��
	float& operator [](int i) 
	{
		assert(0 <= i && i < 3);
		switch (i)
		{
		case 0: return x;
		case 1: return y;
		case 2: return z;
		}
	}
	const float& operator [](int i) const
	{
		assert(0 <= i && i < 3);
		switch (i)
		{
		case 0: return x;
		case 1: return y;
		case 2: return z;
		}
	}

	// �g�ݍ��킹������Z�q
	Vector3& operator +=(const Vector3& a)
	{ x += a.x; y += a.y; z += a.z; return *this; }
	
	Vector3& operator -=(const Vector3& a)
	{ x -= a.x; y -= a.y; z -= a.z; return *this; }
	
	Vector3& operator *=(const Vector3& a) // �v�f���Ƃ̏�Z�i���ςł͂Ȃ���j
	{ x *= a.x; y *= a.y; z *= a.z; return *this; }
	
	Vector3& operator /=(const Vector3& a)
	{
		if (a.x == 0 || a.y == 0 || a.z == 0) return *this;
		x /= a.x; y /= a.y; z /= a.z; return *this; 
	}

	Vector3& operator *=(float s) 
	{ x *= s; y *= s; z *= s; return *this;}
	Vector3& operator /=(float s) 
	{ x /= s; y /= s; z /= s; return *this;}
	
	// �x�N�g���𐳋K������
	void normalize()
	{
		float magSq = x * x + y * y + z * z;
		if (magSq > 0.0f)
		{
			float oneOverMag = 1.0f / sqrtf(magSq);
			x *= oneOverMag;
			y *= oneOverMag;
			z *= oneOverMag;
		}
	}

	// �x�N�g���̐��K���̃R�s�[��Ԃ�
	Vector3 normalized() const
	{
		float magSq = x * x + y * y + z * z;
		if (magSq > 0.0f)
		{
			float oneOverMag = 1.0f / sqrtf(magSq);
			return Vector3(x * oneOverMag, y * oneOverMag, z * oneOverMag);
		}
		return Vector3(0, 0, 0);
	}

	// �x�N�g���̊O��
	static Vector3 Cross(const Vector3& a, const Vector3& b)
	{
		return Vector3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}
	static Vector3 CrossLH(const Vector3& a, const Vector3& b) { return -Cross(a, b); }
	// �x�N�g���̓���
	static float Dot(const Vector3& a, const Vector3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	// �x�N�g���̒�����Ԃ�
	float length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	// �x�N�g���̒����̓���Ԃ�
	float lengthSq() const
	{
		return x * x + y * y + z * z;
	}

	// ���o�w���p
	static void Printf(const Vector3& v, std::string s = "noname")
	{
		printf("%s : {%f, %f, %f}\n", s.c_str(), v.x, v.y, v.z);
	}


};

inline Vector3 operator *(float s, const Vector3& v)
{
	return Vector3(v.x * s, v.y * s, v.z * s);
}

// �C�ӁF�X�J���[���e�����Ŋ���������p�r�i�Ⴆ�� reciprocal �̂悤�Ȏg�����j
inline Vector3 operator /(float s, const Vector3& v)
{
	return Vector3(s / v.x, s / v.y, s / v.z); // �C�Ӂi����ȗp�r�����j
}


#endif