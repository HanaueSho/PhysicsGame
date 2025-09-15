/*
	TowerObject.cpp
	20250914 hanaue sho
	タワーのオブジェクト
*/
#include "TowerObject.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "renderer.h"
#include "texture.h"  // Texture::Load 既存

#include "Manager.h"
#include "Scene.h"
#include "CubeObject.h"
#include "BlockComponent.h"

void TowerObject::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0, 0, 0 });
	tf->SetScale({ 1.0f, 1.0f, 1.0f });
	tf->SetEulerAngles({ 0, 0, 0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateCube(mf, { {1, 1, 1} });

	// 3) Material を追加（シェーダ/テクスチャ/マテリアル）
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\pixelLightingVS.cso");
	Renderer::CreatePixelShader(&ps, "shader\\pixelLightingPS.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\kirby.png");
	// サンプラーは Renderer::Init() で 0番に PSSetSamplers 済みなら null でも描ける
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

	MATERIAL m{};
	m.Diffuse = XMFLOAT4(1, 1, 1, 1);
	m.Ambient = XMFLOAT4(1, 1, 1, 1);
	m.TextureEnable = TRUE;
	mat->SetMaterial(m);

	// 透明テクスチャの可能性が高いのでアルファブレンドに
	mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);

	// 4) MeshRenderer を追加（描画実行係）
	//AddComponent<MeshRendererComponent>();
}

void TowerObject::Update(float dt)
{
	GameObject::Update(dt);

}

void TowerObject::CreateTable(const Vector3& worldPosition)
{
	Vector3 sizeTable = { 7, 0.5f, 2.5f };
	// テーブル生成
	m_pTable = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pTable->Init();
	m_pTable->Transform()->SetScale(sizeTable);
	m_pTable->Transform()->SetPosition(worldPosition);
	m_pTable->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static);

	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\cannon.png");
	m_pTable->GetComponent<MaterialComponent>()->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
}

void TowerObject::CreateTower(int index, const Vector3& offset)
{
	Vector3 size = m_pTable->Transform()->Scale();
	Vector3 sizeBlock = { 1, 1, 1 };
	int num = 0;

	// タワー部分
	for (int i = index; i < index + 3; i++)
	{
		float weight = (float)pow(0.75f, num);
		m_pBlocks[i] = Manager::GetScene()->AddGameObject<CubeObject>(1);
		m_pBlocks[i]->Init();
		m_pBlocks[i]->Transform()->SetScale({ sizeBlock.x * weight, sizeBlock.y * weight, sizeBlock.z * weight });
		m_pBlocks[i]->Transform()->SetPosition({ 0, 0.0f + size.y * 0.5f + sizeBlock.y  * (num + 0.5f), 0 });
		m_pBlocks[i]->AddComponent<BlockComponent>();
		Rigidbody* rigid = m_pBlocks[i]->GetComponent<Rigidbody>();
		rigid->SetBodyType(Rigidbody::BodyType::Dynamic);
		rigid->SetGravityScale(1.0f);
		rigid->SetMass(1.0f * weight);
		rigid->ComputeBoxInertia(m_pBlocks[i]->Transform()->Scale(), rigid->Mass());

		num++;
	}

	// 最後に場所を指定
	Vector3 worldPosition = m_pTable->Transform()->Position() + offset;
	for (int i = index; i < index + 3; i++)
		m_pBlocks[i]->Transform()->SetPosition(m_pBlocks[i]->Transform()->Position() + worldPosition);

	// テクスチャ反映
	auto* srv = Texture::Load("assets\\texture\\block.png");
	for (int i = index; i < index + 3; i++)
	{
		auto* mat = m_pBlocks[i]->GetComponent<MaterialComponent>();
		mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
	}

}

void TowerObject::CreateBridge()
{
}

bool TowerObject::CheckBlocks()
{
	bool b = true;

	for (int i = 0; i < 9; i++)
	{
		if (!m_pBlocks[i]) continue;

		if (!m_pBlocks[i]->GetComponent<BlockComponent>()->IsCounted()) // 一つでもまだ false
			return false;
	}

	return b;
}

