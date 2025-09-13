/*
	PaperPolygone.h
	両面にテクスチャを貼れるペラペラのポリゴン
	20241229 hanaue sho
*/
#ifndef PAPER_H_
#define PAPER_H_

#include <DirectXMath.h>
using namespace DirectX;

class PaperPolygon
{
private:
	bool m_IsSkeleton = false;
	XMFLOAT4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };

public:
	PaperPolygon();
	~PaperPolygon();

	void Update() {}
	void Draw() const;

	void SetIsSkeleton(bool flag) { m_IsSkeleton = flag; }
	void SetColor(const XMFLOAT4& color) { m_Color = color; }
	const XMFLOAT4& GetColor() const { return m_Color; }
};















#endif