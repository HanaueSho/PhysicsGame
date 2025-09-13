/*
	Spherer.h
	20250108 hanaue sho
*/
#ifndef SPHEREREVERSE_H_
#define SPHEREREVERSE_H_
#include "GameObject.h"

#include <DirectXMath.h>
using namespace DirectX;

class SpherePolygonReverse
{
private:
	int m_DivisionH = 4; // 水平分割数
	int m_DivisionV = 4; // 垂直分割数
	float m_Radius = 0.5f; // 半径 （実際は使ってないよ）
	bool m_IsSkeleton = false; // ワイヤーフレーム表示
	XMFLOAT4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool m_IsReverse = false;

public:
	SpherePolygonReverse() = delete;
	SpherePolygonReverse(int division);
	~SpherePolygonReverse();

	void Update() {}
	void Draw() const;

	void SetIsSkeleton(bool flag) { m_IsSkeleton = flag; }
	void SetColor(const XMFLOAT4& color) { m_Color = color; }
	const XMFLOAT4& GetColor() const { return m_Color; }
};






#endif
