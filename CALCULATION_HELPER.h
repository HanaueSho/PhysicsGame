// ----------------------------------------
// CALCULATION_HELPER.h
// 便利な計算機
// 制作日 2024/12/28 -- hanaue sho
// 更新日 2024/12/28 -- hanaue sho
// ----------------------------------------
#ifndef CALCULATION_HELPER_H_
#define CALCULATION_HELPER_H_
#include "DirectXMath.h"
using namespace DirectX;

#include "XMFLOAT_HELPER.h"

// プロトタイプ宣言
XMFLOAT3 Normalize(const XMFLOAT3& vector);
float InnerProduct(const XMFLOAT3& vec1, const XMFLOAT3& vec2);
XMFLOAT3 CrossProduct(const XMFLOAT3& vec1, const XMFLOAT3& vec2);
float LengthFloat3(const XMFLOAT3& f);
float LengthFloat2(const XMFLOAT2& f);
int Sign(const float f);

// --------------------------------------------------
// 正規化
// XMFLOAT3 を正規化する関数
// --------------------------------------------------
static XMFLOAT3 Normalize(const XMFLOAT3& vector) // 正規化
{
	if (vector.x == 0 && vector.y == 0 && vector.z == 0) return { 0.0f, 0.0f, 0.0f };
	float length = sqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
	XMFLOAT3 vec = {};
	vec.x = vector.x / length;
	vec.y = vector.y / length;
	vec.z = vector.z / length;

	return vec;
}
static XMFLOAT2 Normalize(const XMFLOAT2& vector) // 正規化
{
	if (vector.x == 0 && vector.y == 0 ) return { 0.0f, 0.0f };
	float length = sqrt((vector.x * vector.x) + (vector.y * vector.y));
	XMFLOAT2 vec = {};
	vec.x = vector.x / length;
	vec.y = vector.y / length;

	return vec;
}
// --------------------------------------------------
// 内積
// XMFLOAT3 同士の内積を返す関数
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
// 外積
// XMFLOAT3 同士の外積を返す関数
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
// クランプ
// 与えられた値を最小値と最大値の中に収める
// 第１引数　：　値
// 第２引数　：　最小値
// 第３引数　：　最大値
// 返り値　　：　クランプした値
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
// ３つのXMFLOAT3から回転行列を出力
// 第１引数　：　右ベクトル
// 第２引数　：　上ベクトル
// 第３引数　：　前ベクトル
// 返り値　　：　回転行列
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

// 備忘録 ----------

/*
	DirectXの回転行列について
	XMMATRIX が行ベースなのか列ベースなのか不明瞭
	ChatGPTは列ベースと言っているがどうなのか。

	{
		vecR.x, vecT.x, vecF.x, 0.0f,
		vecR.y, vecT.y, vecF.y, 0.0f,
		vecR.z, vecT.z, vecF.z, 0.0f,
		0.0f,   0.0f,   0.0f,   1.0f
	};
*/


// --------------------------------------------------
// ベクトルの長さ
// XMFLOAT3 の指すベクトルの長さを返す
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
// 符号を返す
// 引数の符号を返す（※０は＋を返す）
// --------------------------------------------------
static int Sign(const float f)
{
	if (f < 0)
		return -1;
	else 
		return 1;
}

// --------------------------------------------------
// ２つのベクトルの成す角を返す
// 返り値はラジアン値
// --------------------------------------------------
static float RadianByVectors(const XMFLOAT2& vec1, const XMFLOAT2& vec2)
{
	if (LengthFloat2(vec1) == 0 || LengthFloat2(vec2) == 0) return 0.0f; // セーフティ

	float cos = InnerProduct(vec1, vec2) / (LengthFloat2(vec1) * LengthFloat2(vec2));
	cos = Clamp(cos, -1.0f, 1.0f);
	return acosf(cos);
}





#endif