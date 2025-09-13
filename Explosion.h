/*
	explosion.h
	20250611 hanaue sho
*/
#ifndef EXPLOSION_H_
#define EXPLOSION_H_

#include "main.h"
#include "gameObject.h"

class Explosion : public GameObject
{
private:
	ID3D11Buffer* m_pVertexBuffer;

	ID3D11InputLayout* m_pVertexLayout;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader* m_pPixelShader;

	ID3D11ShaderResourceView* m_pTexture;

	// アニメーション用
	int m_Frame;

public:
	void Init();
	void Uninit();
	void Update();
	void Draw();

};

#endif