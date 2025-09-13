// ----------------------------------------
// CALCULATION_HELPER.h
// �֗��Ȍv�Z�@
// ����� 2024/12/28 -- hanaue sho
// �X�V�� 2024/12/28 -- hanaue sho
// ----------------------------------------
#ifndef CALCULATION_HELPER_H_
#define CALCULATION_HELPER_H_
#include "DirectXMath.h"
using namespace DirectX;

#include "XMFLOAT_HELPER.h"

// �v���g�^�C�v�錾
XMFLOAT3 Normalize(const XMFLOAT3& vector);
float InnerProduct(const XMFLOAT3& vec1, const XMFLOAT3& vec2);
XMFLOAT3 CrossProduct(const XMFLOAT3& vec1, const XMFLOAT3& vec2);
float LengthFloat3(const XMFLOAT3& f);
float LengthFloat2(const XMFLOAT2& f);
int Sign(const float f);

// --------------------------------------------------
// ���K��
// XMFLOAT3 �𐳋K������֐�
// --------------------------------------------------
static XMFLOAT3 Normalize(const XMFLOAT3& vector) // ���K��
{
	if (vector.x == 0 && vector.y == 0 && vector.z == 0) return { 0.0f, 0.0f, 0.0f };
	float length = sqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
	XMFLOAT3 vec = {};
	vec.x = vector.x / length;
	vec.y = vector.y / length;
	vec.z = vector.z / length;

	return vec;
}
static XMFLOAT2 Normalize(const XMFLOAT2& vector) // ���K��
{
	if (vector.x == 0 && vector.y == 0 ) return { 0.0f, 0.0f };
	float length = sqrt((vector.x * vector.x) + (vector.y * vector.y));
	XMFLOAT2 vec = {};
	vec.x = vector.x / length;
	vec.y = vector.y / length;

	return vec;
}
// --------------------------------------------------
// ����
// XMFLOAT3 ���m�̓��ς�Ԃ��֐�
// --------------------------------------------------
static float InnerProduct(const XMFLOAT3& vec1, const XMFLOAT3& vec2)
{
	return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z);
}
static float InnerProduct(const XMFLOAT2& vec1, const XMFLOAT2& vec2)
{
	return (vec1.x * vec2.x) + (vec1.y * vec2.y);
}

// --------------------------------------------------
// �O��
// XMFLOAT3 ���m�̊O�ς�Ԃ��֐�
// --------------------------------------------------
static XMFLOAT3 CrossProduct(const XMFLOAT3& vec1, const XMFLOAT3& vec2)
{
	return
	{
		vec1.y * vec2.z - vec1.z * vec2.y,
		vec1.z * vec2.x - vec1.x * vec2.z,
		vec1.x * vec2.y - vec1.y * vec2.x
	};
}

// --------------------------------------------------
// �N�����v
// �^����ꂽ�l���ŏ��l�ƍő�l�̒��Ɏ��߂�
// ��P�����@�F�@�l
// ��Q�����@�F�@�ŏ��l
// ��R�����@�F�@�ő�l
// �Ԃ�l�@�@�F�@�N�����v�����l
// --------------------------------------------------
static float Clamp(const float& value, const float& min, const float& max)
{
	if (value <= min)
		return min;
	if (value >= max)
		return max;
	return value;
}
static XMFLOAT2 Clamp(const XMFLOAT2& value, const float& min, const float& max, const bool& isLength = true)
{
	if (isLength)
	{
		float length = LengthFloat2(value);
		if (length > max)
			return Normalize(value) * max;
		if (length < min)
			return Normalize(value) * min;
		return value;
	}
	return { Clamp(value.x, min, max), Clamp(value.y, min, max) };
}
static XMFLOAT3 Clamp(const XMFLOAT3& value, const float& min, const float& max, const bool& isLength = true)
{
	if (isLength)
	{
		float length = LengthFloat3(value);
		if (length > max)
			return Normalize(value) * max;
		if (length < min)
			return Normalize(value) * min;
		return value;
	}
	return { Clamp(value.x, min, max), Clamp(value.y, min, max), Clamp(value.z, min, max) };
}


// --------------------------------------------------
// �R��XMFLOAT3�����]�s����o��
// ��P�����@�F�@�E�x�N�g��
// ��Q�����@�F�@��x�N�g��
// ��R�����@�F�@�O�x�N�g��
// �Ԃ�l�@�@�F�@��]�s��
// --------------------------------------------------
static XMMATRIX RotateMatrixFromVector3(const XMFLOAT3& vecR, const XMFLOAT3& vecT, const XMFLOAT3& vecF)
{
	return
	{
		vecR.x, vecR.y, vecR.z, 0.0f,
		vecT.x, vecT.y, vecT.z, 0.0f,
		vecF.x, vecF.y, vecF.z, 0.0f,
		0.0f,   0.0f,   0.0f,   1.0f
	};
}

// ���Y�^ ----------

/*
	DirectX�̉�]�s��ɂ���
	XMMATRIX ���s�x�[�X�Ȃ̂���x�[�X�Ȃ̂��s����
	ChatGPT�͗�x�[�X�ƌ����Ă��邪�ǂ��Ȃ̂��B

	{
		vecR.x, vecT.x, vecF.x, 0.0f,
		vecR.y, vecT.y, vecF.y, 0.0f,
		vecR.z, vecT.z, vecF.z, 0.0f,
		0.0f,   0.0f,   0.0f,   1.0f
	};
*/


// --------------------------------------------------
// �x�N�g���̒���
// XMFLOAT3 �̎w���x�N�g���̒�����Ԃ�
// --------------------------------------------------
static float LengthFloat3(const XMFLOAT3& f)
{
	return sqrt(f.x * f.x + f.y * f.y + f.z * f.z);
}
static float LengthFloat2(const XMFLOAT2& f)
{
	return sqrt(f.x * f.x + f.y * f.y);
}

// --------------------------------------------------
// ������Ԃ�
// �����̕�����Ԃ��i���O�́{��Ԃ��j
// --------------------------------------------------
static int Sign(const float f)
{
	if (f < 0)
		return -1;
	else 
		return 1;
}

// --------------------------------------------------
// �Q�̃x�N�g���̐����p��Ԃ�
// �Ԃ�l�̓��W�A���l
// --------------------------------------------------
static float RadianByVectors(const XMFLOAT2& vec1, const XMFLOAT2& vec2)
{
	if (LengthFloat2(vec1) == 0 || LengthFloat2(vec2) == 0) return 0.0f; // �Z�[�t�e�B

	float cos = InnerProduct(vec1, vec2) / (LengthFloat2(vec1) * LengthFloat2(vec2));
	cos = Clamp(cos, -1.0f, 1.0f);
	return acosf(cos);
}





#endif