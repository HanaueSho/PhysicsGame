/*
	PaperPolygone.cpp
	両面にテクスチャを貼れるペラペラのポリゴン
	20241229 hanaue sho
*/
#include "PaperPolygon.h"
#include "renderer.h"
#include "texture.h"

static ID3D11Buffer* g_VertexBuffer = NULL; // 頂点情報
static ID3D11Buffer* g_IndexBuffer = NULL; // インデックス情報
static ID3D11ShaderResourceView* g_pTextureId = nullptr; // テクスチャID

PaperPolygon::PaperPolygon()
{
	ID3D11Device* pDevice = Renderer::GetDevice();

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {}; // ZeroMemory と同じ初期化
	bd.Usage = D3D11_USAGE_DEFAULT; // 
	bd.ByteWidth = sizeof(VERTEX_3D) * 8; // バッファサイズ　構造体サイズ×頂点数分 １面当たり三角形２つ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// 頂点配列
	VERTEX_3D vertex[8];

	// 表面
	vertex[0].Position = { -0.5f, 0.5f, 0.0f }; // 左上
	vertex[0].TexCoord = { 0.0f, 0.0f };
	vertex[0].Normal = { 0.0f, 0.0f, -1.0f };
	vertex[0].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[1].Position = { 0.5f, 0.5f, 0.0f }; // 右上
	vertex[1].TexCoord = { 1.0f, 0.0f };
	vertex[1].Normal = { 0.0f, 0.0f, -1.0f };
	vertex[1].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[2].Position = { -0.5f, -0.5f, 0.0f }; // 左下
	vertex[2].TexCoord = { 0.0f, 1.0f };
	vertex[2].Normal = { 0.0f, 0.0f, -1.0f };
	vertex[2].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[3].Position = { 0.5f, -0.5f, 0.0f }; // 右下
	vertex[3].TexCoord = { 1.0f, 1.0f };
	vertex[3].Normal = { 0.0f, 0.0f, -1.0f };
	vertex[3].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	// 裏面
	vertex[4].Position = { 0.5f, 0.5f, 0.0f }; // 右上
	vertex[4].TexCoord = { 0.0f, 0.0f };
	vertex[4].Normal = { 0.0f, 0.0f, 1.0f };
	vertex[4].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[5].Position = { -0.5f, 0.5f, 0.0f }; // 左上
	vertex[5].TexCoord = { 1.0f, 0.0f };
	vertex[5].Normal = { 0.0f, 0.0f, 1.0f };
	vertex[5].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[6].Position = { 0.5f, -0.5f, 0.0f }; // 右下
	vertex[6].TexCoord = { 0.0f, 1.0f };
	vertex[6].Normal = { 0.0f, 0.0f, 1.0f };
	vertex[6].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[7].Position = { -0.5f, -0.5f, 0.0f }; // 左下
	vertex[7].TexCoord = { 1.0f, 1.0f };
	vertex[7].Normal = { 0.0f, 0.0f, 1.0f };
	vertex[7].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	// インデックス配列
	unsigned short indexes[12] =
	{
		0, 1, 2,
		1, 3, 2,
		4, 5, 6,
		5, 7, 6
	};
	

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertex;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	Renderer::GetDevice()->CreateBuffer(&bd, &data, &g_VertexBuffer);

	// インデックスバッファ生成
	// D3D11_BUFFER_DESC bd = {}; // ZeroMemory と同じ初期化 使いまわす
	// bd.Usage = D3D11_USAGE_DEFAULT; // 使いまわしてるので書かなくてよい
	bd.ByteWidth = sizeof(unsigned short) * 12; // バッファサイズ　構造体サイズ×頂点数分 １面当たり三角形２つ
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER; // 指定
	// bd.CPUAccessFlags = 0; // 使いまわしてる

	// D3D11_SUBRESOURCE_DATA data; // 使いまわし
	data.pSysMem = indexes;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	Renderer::GetDevice()->CreateBuffer(&bd, &data, &g_IndexBuffer); // 頂点バッファを作る

	// テクスチャ読み込み
	g_pTextureId = Texture::Load("asset\\texture\\whiteTexture.png");
}

PaperPolygon::~PaperPolygon()
{
	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
	if (g_IndexBuffer)
	{
		g_IndexBuffer->Release();
		g_IndexBuffer = NULL;
	}
}

void PaperPolygon::Draw() const
{
	// テクスチャの読み込み
	//ID3D11ShaderResourceView* srv = GetTexture(g_textureId);
	//GetDeviceContext()->PSSetShaderResources(0, 1, &srv);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D); // 頂点のサイズ
	UINT offset = 0; // 頂点バッファ上のスタートの情報
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); // g_VertexBuffer 表示する頂点バッファーのポインター

	// インデックスバッファ設定
	Renderer::GetDeviceContext()->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// プリミティブトポロジ設定
	if (!m_IsSkeleton)
		Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	else
		Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = GetColor();
	Renderer::SetMaterial(material);

	// ポリゴン描画
	//GetDeviceContext()->Draw(8, 0); // Draw の第１引数は表示する頂点数の数
	Renderer::GetDeviceContext()->DrawIndexed(12, 0, 0); // インデックスでの描画
	//GetDeviceContext()->DrawIndexed(36, 0, 0); // インデックスでの描画
}
