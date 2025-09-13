/*
	Matrix4x4.h
	20250806  hanaue sho
	行ベクトル流儀（４行目に平行移動成分）
*/
#ifndef MATRIX4X4_H_
#define MATRIX4X4_H_
#include <cstring>
#include <math.h>
#include "Vector3.h"

class Matrix4x4
{
public:
	float m[4][4]; // 行列本体

	Matrix4x4() { identity(); }

	// 単位行列に初期化
	void identity()
	{
		memset(m, 0, sizeof(m)); // 初期化
		m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
	}

	// 左上3x3のみをゼロに
	void Zero3x3()
	{
		memset(m, 0, sizeof(m)); // 初期化
		m[3][3] = 1.0f; // [3][3]のみ１，それ以外は０に
	}

	// 行列同士の掛け算
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

	// 行列と float の掛け算（各要素に float を掛ける）
	Matrix4x4 operator *(float f) const
	{
		Matrix4x4 result;
		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
			{
				result.m[row][col] = m[row][col] * f; // 各成分に float を掛ける
			}
		}
		return result;
	}

	// 平行移動行列
	static Matrix4x4 CreateTranslation(float x, float y, float z)
	{
		Matrix4x4 mat;
		mat.identity();
		mat.m[3][0] = x;
		mat.m[3][1] = y;
		mat.m[3][2] = z;
		return mat;
	}

	// スケール行列
	static Matrix4x4 CreateScale(float sx, float sy, float sz)
	{
		Matrix4x4 mat;
		mat.identity();
		mat.m[0][0] = sx;
		mat.m[1][1] = sy;
		mat.m[2][2] = sz;
		return mat;
	}

	// 回転行列 （Yaw→Pitch→Roll）全てラジアン（左手座標系）
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

	// 回転行列（軸、角度から回転行列を作る）
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

	// Transform からMatrix を生成
	static Matrix4x4 CreateTRS(const Vector3& pos, const Matrix4x4& rot3x3, const Vector3& s)
	{
		Matrix4x4 M = rot3x3;
		// 行スケール (S * R)
		M.m[0][0] *= s.x; M.m[0][1] *= s.x; M.m[0][2] *= s.x;
		M.m[1][0] *= s.y; M.m[1][1] *= s.y; M.m[1][2] *= s.y;
		M.m[2][0] *= s.z; M.m[2][1] *= s.z; M.m[2][2] *= s.z;
		// 平行移動 (S * R * T)
		M.m[3][0] = pos.x; M.m[3][1] = pos.y; M.m[3][2] = pos.z;
		M.m[0][3] = M.m[1][3] = M.m[2][3] = 0.0f; M.m[3][3] = 1.0f;
		return M;
	}

	// 転置処理
	Matrix4x4 Transpose() const 
	{
		Matrix4x4 t;
		for (int r = 0; r < 4; ++r)
			for (int c = 0; c < 4; ++c)
				t.m[r][c] = m[c][r];
		return t;
	}

	// 逆行列
	Matrix4x4 Inverse() const
	{
		Matrix4x4 inv;
		inv.identity();

		// 3x3 の逆行列を計算（回転＋スケール部分）
		float det =
			m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
			m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
			m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

		if (fabs(det) < 1e-8f) {
			// 逆行列が存在しない
			return inv; // 単位行列を返すなど適宜エラー処理
		}

		// 長さで割る
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

		// 平行移動の逆を計算
		inv.m[3][0] = -(m[3][0] * inv.m[0][0] + m[3][1] * inv.m[1][0] + m[3][2] * inv.m[2][0]);
		inv.m[3][1] = -(m[3][0] * inv.m[0][1] + m[3][1] * inv.m[1][1] + m[3][2] * inv.m[2][1]);
		inv.m[3][2] = -(m[3][0] * inv.m[0][2] + m[3][1] * inv.m[1][2] + m[3][2] * inv.m[2][2]);

		return inv;
	}

	// 行列からオイラー角へ変換
	Vector3 ToEulerAngles() const
	{
		Vector3 euler;

		// pitch(X軸回転)
		if (fabs(m[2][1]) < 0.9999f)
		{
			euler.x = asinf(m[2][1]); // pitch
			euler.y = atan2f(-m[2][0], m[2][2]); // yaw
			euler.z = atan2f(-m[0][1], m[1][1]); // roll
		}
		else
		{
			// ジンバルロック（m[2][1] ≒ ±１）の処理
			euler.x = (m[2][1] > 0.0f) ? PI / 2 : -PI / 2;
			euler.y = atan2f(m[0][2], m[0][0]);
			euler.z = 0.0f;
		}
		return euler;
	}

	// ベクトルに対して回転行列を適用
	Vector3 TransformNormal(const Vector3& vect) const
	{
		// [行][列]
		return Vector3(
			vect.x * m[0][0] + vect.y * m[1][0] + vect.z * m[2][0],
			vect.x * m[0][1] + vect.y * m[1][1] + vect.z * m[2][1],
			vect.x * m[0][2] + vect.y * m[1][2] + vect.z * m[2][2]
		);
	}

	// ベクトルに対して回転＋平行移動を適用（ワールド座標変換）
	Vector3 TransformPoint(const Vector3& vect) const
	{
		return Vector3(
			vect.x * m[0][0] + vect.y * m[1][0] + vect.z * m[2][0] + m[3][0],
			vect.x * m[0][1] + vect.y * m[1][1] + vect.z * m[2][1] + m[3][1],
			vect.x * m[0][2] + vect.y * m[1][2] + vect.z * m[2][2] + m[3][2]
		);
	}

	// 行列から回転情報のみを取り出す
	Matrix4x4 RotationNormalized() const
	{
		// 各行成分を取り出す
		Vector3 r0(m[0][0], m[0][1], m[0][2]);
		Vector3 r1(m[1][0], m[1][1], m[1][2]);
		Vector3 r2(m[2][0], m[2][1], m[2][2]);

		// 各行のスケールを取り出す
		float sx = r0.length(); if (sx < 1e-8f) { r0 = Vector3(1, 0, 0); sx = 1; } else {r0 *= (1.0f / sx); }
		float sy = r1.length(); if (sy < 1e-8f) { r1 = Vector3(0, 1, 0); sy = 1; } else {r1 *= (1.0f / sy); }
		float sz = r2.length(); if (sz < 1e-8f) { r2 = Vector3(0, 0, 1); sz = 1; } else {r2 *= (1.0f / sz); }

		// 反転検出
		if (Vector3::Dot(Vector3::Cross(r0, r1), r2) < 0.0f) r2 *= -1.0f;

		// 正規化された回転情報を返す
		Matrix4x4 R; R.identity();
		R.m[0][0] = r0.x; R.m[0][1] = r0.y; R.m[0][2] = r0.z;
		R.m[1][0] = r1.x; R.m[1][1] = r1.y; R.m[1][2] = r1.z;
		R.m[2][0] = r2.x; R.m[2][1] = r2.y; R.m[2][2] = r2.z;
		return R;
	}

};

#endif