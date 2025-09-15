/*
	FadeObject.cpp
	20250915 hanaue sho
	平面のオブジェクト
*/
#include "FadeObject.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "renderer.h"
#include "texture.h"  // Texture::Load 既存

#include "BillboardComponent.h"


void FadeObject::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0, 10, 0 });
	tf->SetScale({ 10.0f, 10.0f, 1.0f });
	tf->SetEulerAngles({ 0, 0, 0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateQuad(mf, { 1, 1 });

	// 3) Material を追加（シェーダ/テクスチャ/マテリアル）
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&ps, "shader\\unlitTexturePS.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\textureWhite.png");
	// サンプラーは Renderer::Init() で 0番に PSSetSamplers 済みなら null でも描ける
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

	MATERIAL m{};
	m.Diffuse = XMFLOAT4(0, 0, 0, 1);
	m.Ambient = XMFLOAT4(1, 1, 1, 1);
	m.TextureEnable = TRUE;
	mat->SetMaterial(m);

	// 透明テクスチャの可能性が高いのでアルファブレンドに
	mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);

	// 4) MeshRenderer を追加（描画実行係）
	AddComponent<MeshRendererComponent>();

	BillboardComponent* pBill =AddComponent<BillboardComponent>();
	pBill->SetType(BillboardComponent::BillboardType::CylindericalY);
}

void FadeObject::Update(float dt)
{
	GameObject::Update(dt);

	if (m_FadeMode == Fade::In) // 1 -> 0
	{
		auto* mat = GetComponent<MaterialComponent>();
		MATERIAL m = mat->GetMaterial();
		m.Diffuse.w -= dt / m_FadeTime;
		if (m.Diffuse.w < 0)
		{
			m.Diffuse.w = 0; 
			m_FadeMode = Fade::None;
		}
		mat->SetMaterial(m);
	}
	else if (m_FadeMode == Fade::Out) // 0 -> 1
	{
		auto* mat = GetComponent<MaterialComponent>();
		MATERIAL m = mat->GetMaterial();
		m.Diffuse.w += dt / m_FadeTime;
		if (m.Diffuse.w > 1)
		{
			m.Diffuse.w = 1;
			m_FadeMode = Fade::None;
		}
		mat->SetMaterial(m);
	}
}

