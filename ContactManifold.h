/*
	ContactManifold.h
	20250901  hanaue sho
	接触点の情報の構造体
*/
#ifndef CONTACTMANIFOLD_H_
#define CONTACTMANIFOLD_H_
#include"Vector3.h"

struct ContactPoint
{
	Vector3 pointOnA{}; // Aの接触点
	Vector3 pointOnB{}; // Bの接触点
	float penetration = 0.0f; // 貫入深度
	float accumN = 0.0f; // 法線の累積正規インパルス（１軸ならスカラーで良い）
	Vector3 accumImpulseT = Vector3(); // 接線の累積正規インパルス（１軸ならスカラーで良い）
};

struct ContactManifold
{
	static constexpr int MAX_POINTS = 4; // OBB*OBB なので４つに
	bool touching = false; // 触れているか判定
	Vector3 normal{};		// A → B への単位ベクトル
	int count = 0; // 有効な接触点の数
	ContactPoint points[4]; // 最大４点
};

#endif