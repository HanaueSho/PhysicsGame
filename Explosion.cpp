/*
	explosion.cpp
	20250611 hanaue sho
*/
#include "explosion.h"
#include "main.h"
#include "renderer.h"
#include "Manager.h"
#include "camera.h"
#include "texture.h"
#include "scene.h"

void Explosion::Init()
{
	VERTEX_3D vertex[4];

	vertex[0].Position = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(1.0f, 1.0f, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(1.0f, -1.0f, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
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
	//TexMetadata metadata;
	//ScratchImage image;
	//LoadFromWICFile(L"assets\\texture\\explosion.png", WIC_FLAGS_NONE, &metadata, image);
	////LoadFromWICFile(L"assets\\texture\\HumanTube.png", WIC_FLAGS_NONE, &metadata, image);
	//CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &m_pTexture);
	//assert(m_pTexture);

	m_pTexture = Texture::Load("assets\\texture\\explosion.png");

	Renderer::CreateVertexShader(&m_pVertexShader, &m_pVertexLayout, "shader\\unlitTextureVS.cso");

	Renderer::CreatePixelShader(&m_pPixelShader, "shader\\unlitTexturePS.cso");

	// 初期化処理
	m_Frame = 0;

}

void Explosion::Uninit()
{
	m_pVertexBuffer->Release();
	m_pVertexShader->Release();
	m_pPixelShader->Release();
	m_pVertexLayout->Release();
	//m_pTexture->Release();
}

void Explosion::Update()
{
	/*
	m_Frame++;
	if (m_Frame > 15) SetDestroy();
	*/
}

void Explosion::Draw()
{
	/*
	// 頂点データ書き換え
	D3D11_MAPPED_SUBRESOURCE msr;
	Renderer::GetDeviceContext()->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float textureWidth  = 1.0f / 4; // テクスチャの横の数
	float textureHeight = 1.0f / 4; // テクスチャの縦の数
	float textureX = (m_Frame % 4) * textureWidth; // テクスチャの横座標
	float textureY = (m_Frame / 4) * textureHeight; // テクスチャの縦座標

	vertex[0].Position = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(textureX, textureY);

	vertex[1].Position = XMFLOAT3(1.0f, 1.0f, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(textureX + textureWidth, textureY);

	vertex[2].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(textureX, textureY + textureHeight);

	vertex[3].Position = XMFLOAT3(1.0f, -1.0f, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(textureX + textureWidth, textureY + textureHeight);

	Renderer::GetDeviceContext()->Unmap(m_pVertexBuffer, 0);


	// 入力レイアウト
	Renderer::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

	// シェーダー設定
	Renderer::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

	// マトリクス設定
	XMMATRIX world, scale, rotate, translation;
	scale = XMMatrixScaling(m_Transform.scale.x, m_Transform.scale.y, m_Transform.scale.z);
	rotate = XMMatrixRotationRollPitchYaw(m_Transform.rotation.ToEulerAngles().x, m_Transform.rotation.ToEulerAngles().y, m_Transform.rotation.ToEulerAngles().z);
	translation = XMMatrixTranslation(m_Transform.position.x, m_Transform.position.y, m_Transform.position.z);

	// ビルボードマトリクス
	XMMATRIX viewMtxInverse = XMMatrixTranspose(Manager::GetScene()->GetGameObject<Camera>()->GetMatrixView()); // 逆行列を転置変換により得る
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, viewMtxInverse);
	matrix._14 = matrix._24 = matrix._34 = 0.0f; // 平行移動成分をカット（転置されてるのでここになる）
	XMMATRIX billboardMatrix = XMLoadFloat4x4(&matrix);

	world = scale * rotate * billboardMatrix * translation;
	Renderer::SetWorldMatrix(world);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// テクスチャ設定
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_pTexture);

	// マテリアル設定
	MATERIAL material{};
	material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画
	Renderer::GetDeviceContext()->Draw(4, 0);
	*/
}
