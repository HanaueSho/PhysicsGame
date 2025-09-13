/*
	score.h
	20250702 hanaue sho
*/
#ifndef SCORE_H_
#define SCORE_H_

#include "main.h"
#include "gameObject.h"

class Score : public GameObject
{
private:
	ID3D11Buffer* m_pVertexBuffer;

	ID3D11InputLayout* m_pVertexLayout;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader* m_pPixelShader;

	ID3D11ShaderResourceView* m_pTexture;


	int m_Value;

public:
	void Init();
	void Init(Vector3 position, Vector3 rotate, Vector3 scale);
	void Uninit();
	void Update();
	void Draw();

	void Add(int value) { m_Value += value; }

};

#endif