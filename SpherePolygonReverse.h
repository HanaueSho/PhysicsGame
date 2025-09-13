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
	int m_DivisionH = 4; // ����������
	int m_DivisionV = 4; // ����������
	float m_Radius = 0.5f; // ���a �i���ۂ͎g���ĂȂ���j
	bool m_IsSkeleton = false; // ���C���[�t���[���\��
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
