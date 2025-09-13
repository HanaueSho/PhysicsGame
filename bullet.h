/*
	bullet.h
	20250514 hanaue sho
*/
#pragma once

#include "main.h"
#include "gameObject.h"

class ModelRenderer;

class Bullet : public GameObject
{
private:
	ID3D11InputLayout* m_pVertexLayout;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader* m_pPixelShader;

	ModelRenderer* m_pModelRenderer;

	ID3D11ShaderResourceView* m_pTexture;

	Vector3 m_TargetVector = Vector3(0, 0, 0); // 目標の正面ベクトル

public:
	void Init();
	void Uninit();
	void Update();
	void Draw();

	float RadianByVector3(Vector3 vect_1, Vector3 vect_2);



};
