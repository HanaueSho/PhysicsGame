/*
	score.cpp
	20250702 hanaue sho
*/
#include "score.h"
#include "main.h"
#include "renderer.h"
#include "texture.h"
#include "MathConversion.h"


void Score::Init()
{
	VERTEX_3D vertex[4];

	vertex[0].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(200.0f, 0.0f, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(0.0f, 200.0f, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(200.0f, 200.0f, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC; // 頂点データを触るので DYNAMIC にする ----------
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // map で頂点データを書き換えられるようにする ----------

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertex;

	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_pVertexBuffer);

	// テクスチャ読み込み
	m_pTexture = Texture::Load("assets\\texture\\number.png");

	Renderer::CreateVertexShader(&m_pVertexShader, &m_pVertexLayout, "shader\\unlitTextureVS.cso");

	Renderer::CreatePixelShader(&m_pPixelShader, "shader\\unlitTexturePS.cso");

	// 初期化
	m_Value = 321;
}

void Score::Init(Vector3 position, Vector3 rotate, Vector3 scale)
{
	Init();
	/*
	m_Transform.position = position;
	m_Transform.rotation = Quaternion::FromEulerAngles(rotate);
	m_Transform.scale = scale;
	*/
}

void Score::Uninit()
{
	m_pVertexBuffer->Release();
	m_pVertexShader->Release();
	m_pPixelShader->Release();
	m_pVertexLayout->Release();

}

void Score::Update()
{
	//m_Value++;
	//m_Value = m_Number % 10;
}

void Score::Draw()
{
	/*
	// 入力レイアウト
	Renderer::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

	// シェーダー設定
	Renderer::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

	// マトリクスの設定
	Renderer::SetWorldViewProjection2D();

	// マトリクス設定
	XMMATRIX world;
	world = ToXMMATRIX(m_Transform.GetWorldMatrix());
	Renderer::SetWorldMatrix(world);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// マテリアル設定
	MATERIAL material{};
	material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	// テクスチャ設定
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_pTexture);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);



	float x = 0.0f;
	float y = 0.0f;
	float w = 50.0f;
	float h = 50.0f;

	int value = m_Value;
	int digit = 0;
	while (value != 0)
	{
		digit++;
		value /= 10;
	}
	value = m_Value;

	float textureWidth = 1.0f / 10; // テクスチャの横の数
	float textureHeight = 1.0f / 1; // テクスチャの縦の数


	for (int i = 0; i < digit; i++)
	{
		//// 頂点データ書き換え
		D3D11_MAPPED_SUBRESOURCE msr;
		Renderer::GetDeviceContext()->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

		VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

		int num = value % 10;
		value /= 10;

		float textureX = (num % 10) * textureWidth; // テクスチャの横座標
		float textureY = (num / 10) * textureHeight; // テクスチャの縦座標

		vertex[0].Position = XMFLOAT3(x, y, 0.0f);
		vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[0].TexCoord = XMFLOAT2(textureX, textureY);

		vertex[1].Position = XMFLOAT3(x + w, y, 0.0f);
		vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[1].TexCoord = XMFLOAT2(textureX + textureWidth, textureY);

		vertex[2].Position = XMFLOAT3(x, y + h, 0.0f);
		vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[2].TexCoord = XMFLOAT2(textureX, textureY + textureHeight);

		vertex[3].Position = XMFLOAT3(x + h, y + h, 0.0f);
		vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[3].TexCoord = XMFLOAT2(textureX + textureWidth, textureY + textureHeight);

		Renderer::GetDeviceContext()->Unmap(m_pVertexBuffer, 0);

		Renderer::GetDeviceContext()->Draw(4, 0);

		x -= 50;
	}

	// ポリゴン描画
	*/
}
