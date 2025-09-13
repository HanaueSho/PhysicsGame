/*
	SpherePolygone.cpp
	20241212 hanaue sho
*/
#include "SpherePolygon.h"
#include "renderer.h"
#include "texture.h"


static ID3D11Buffer* g_VertexBuffer = NULL; // 頂点情報
static ID3D11Buffer* g_IndexBuffer = NULL; // インデックス情報
static ID3D11ShaderResourceView* g_pTextureId = nullptr; // テクスチャID


SpherePolygon::SpherePolygon(int division, bool isReverse)
{
	m_IsReverse = isReverse;
	// 頂点数の計算
	const int divisionH = 12; // 垂直方向分割数
	const int divisionV = 12; // 水平方向分割数
	const int numVertex = divisionV * (divisionH + 1); // 総頂点数
	const int numIndex = (divisionH * 2 + divisionH * (divisionV - 2) * 2) * 3; // インデックス数
	XMFLOAT3 radian = {}; // 角度
	
	m_DivisionH = divisionH; // メンバ変数に格納
	m_DivisionV = divisionV; // メンバ変数に格納


	ID3D11Device* pDevice = Renderer::GetDevice();

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {}; // ZeroMemory と同じ初期化
	bd.Usage = D3D11_USAGE_DEFAULT; // 
	bd.ByteWidth = sizeof(VERTEX_3D) * numVertex; // バッファサイズ　構造体サイズ×頂点数分 １面当たり三角形２つ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// 頂点配列
	VERTEX_3D vertex[numVertex] = {};

	// インデックス配列
	unsigned short indexes[numIndex] = {};
	int count = 0; // カウンター
	
	for (int i = 0; i < divisionH + 1; i++) // 水平方向分割
	{
		radian.y = (180.0f / divisionH) * i * XM_PI / 180; // ｙ座標の角度更新

		for (int j = 0; j < divisionV; j++) // 垂直方向分割
		{
			// 角度更新 -----
			radian.x = (360.0f / divisionV) * j * XM_PI / 180;
			radian.z = (360.0f / divisionV) * j * XM_PI / 180;

			// 頂点の座標計算 -----
			XMFLOAT3 position = {};
			position.y = cosf(radian.y) * 0.5f;
			position.x = sinf(radian.x) * sinf(radian.y) * 0.5f;
			position.z = cosf(radian.z) * sinf(radian.y) * 0.5f;

			vertex[i * divisionV + j].Position = position; // 座標格納

			// テックスコードの設定 -----
			if (i % 2 == 0 && j % 2 == 0)
			{
				vertex[i * divisionV + j].TexCoord = {0.0f, 0.0f};
			}
			else if (i % 2 == 0 && j % 2 == 1)
			{
				vertex[i * divisionV + j].TexCoord = { 1.0f, 0.0f };
			}
			else if (i % 2 == 1 && j % 2 == 0)
			{
				vertex[i * divisionV + j].TexCoord = { 0.0f, 1.0f};
			}
			else if (i % 2 == 1 && j % 2 == 1)
			{
				vertex[i * divisionV + j].TexCoord = { 1.0f, 1.0f };
			}

			// 法線ベクトルの設定 -----
			vertex[i * divisionV + j].Normal = position;
			 
			// 色の設定 -----
			vertex[i * divisionV + j].Diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
			
			// ここで index を処理する -----
			//if (i != 0) // こいつ要らない
			{
				if (i > 1 && i < divisionH) // 一番上と一番下は処理が異なる
				{
					indexes[count] = (i - 1) * divisionV + j; // 上
					indexes[count + 1] = i * divisionV + j; // ここ生成
					indexes[count + 2] = i * divisionV + j + 1; // となり

					indexes[count + 3] = (i - 1) * divisionV + j; // 上
					indexes[count + 4] = i * divisionV + j + 1; // となり
					indexes[count + 5] = (i - 1) * divisionV + j + 1; // 上となり

					if (j == divisionV - 1) // 最後の三角形を補正
					{
						indexes[count + 2] = i * divisionV + j + 1 - divisionV; // となり
						
						indexes[count + 4] = i * divisionV + j + 1 - divisionV; // となり
						indexes[count + 5] = (i - 1) * divisionV + j + 1 - divisionV; // 上となり
					}

					count += 6;
				}
				else if (i == 1) // 一番上
				{
					indexes[count] = (i - 1) * divisionV + j; // 上
					indexes[count + 1] = i * divisionV + j; // ここ生成
					indexes[count + 2] = i * divisionV + j + 1; // となり
					
					if (j == divisionH - 1) // 最後の三角形を補正
					{
						indexes[count + 2] = i * divisionV + j + 1 - divisionV; // となり
					}

					count += 3;
				}
				else if (i == divisionH) // 一番下
				{
					indexes[count] = (i - 1) * divisionV + j; // 上
					indexes[count + 1] = i * divisionV + j; // ここ生成
					indexes[count + 2] = (i - 1) * divisionV + j + 1; // 上となり

					if (j == divisionV - 1) // 最後の三角形を補正
					{
						indexes[count + 2] = (i - 1) * divisionV + j + 1 - divisionV; // 上となり
					}

					count += 3;
				}
			}
		}
	}



	if (m_IsReverse)
	{
		unsigned short dummy = 0;
		for (int i = 0; i < numIndex; i+=3)
		{
			dummy = indexes[i + 1];
			indexes[i + 1] = indexes[i + 2];
			indexes[i + 2] = dummy;

			// テクスチャ座標を反転
			for (int j = 0; j < 3; ++j) 
			{
				vertex[indexes[i + j]].TexCoord.x = 1.0f - vertex[indexes[i + j]].TexCoord.x;
				vertex[indexes[i + j]].Normal.x = -vertex[indexes[i + j]].Normal.x;
				vertex[indexes[i + j]].Normal.y = -vertex[indexes[i + j]].Normal.y;
				vertex[indexes[i + j]].Normal.z = -vertex[indexes[i + j]].Normal.z;
			}
		}
	}

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertex;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	Renderer::GetDevice()->CreateBuffer(&bd, &data, &g_VertexBuffer);

	// インデックスバッファ生成
	// D3D11_BUFFER_DESC bd = {}; // ZeroMemory と同じ初期化 使いまわす
	// bd.Usage = D3D11_USAGE_DEFAULT; // 使いまわしてるので書かなくてよい
	bd.ByteWidth = sizeof(unsigned short) * numIndex; // バッファサイズ　構造体サイズ×頂点数分 １面当たり三角形２つ
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER; // 指定
	// bd.CPUAccessFlags = 0; // 使いまわしてる

	// D3D11_SUBRESOURCE_DATA data; // 使いまわし
	data.pSysMem = indexes;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	Renderer::GetDevice()->CreateBuffer(&bd, &data, &g_IndexBuffer); // 頂点バッファを作る

	// テクスチャ読み込み
	g_pTextureId = Texture::Load("asset\\texture\\kirby.png");

}

SpherePolygon::~SpherePolygon()
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

void SpherePolygon::Draw() const
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
	//GetDeviceContext()->Draw(m_DivisionV * (m_DivisionH + 1), 0); // Draw の第１引数は表示する頂点数の数
	Renderer::GetDeviceContext()->DrawIndexed((m_DivisionH * 2 + m_DivisionH * (m_DivisionV - 2) * 2) * 3, 0, 0); // インデックスでの描画
	//GetDeviceContext()->DrawIndexed(36, 0, 0); // インデックスでの描画

}
