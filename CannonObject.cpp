/*
	CannonObject.cpp
	20250914 hanaue sho
	大砲のオブジェクト
*/
#include "CannonObject.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "renderer.h"
#include "texture.h"  // Texture::Load 既存

#include "Keyboard.h"
#include "Manager.h"
#include "Scene.h"
#include "AppleObject.h"


void CannonObject::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0, 5, 0 });
	tf->SetScale({ 1.0f, 1.0f, 1.0f });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateCylinder(mf, { 1, 1, 12, 12 });

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
	AddComponent<MeshRendererComponent>();

}

void CannonObject::Update(float dt)
{
	GameObject::Update(dt);

	if (Keyboard_IsKeyDown(KK_UP))
	{
		Transform()->Value().RotateAxis({ 1, 0, 0 }, -0.1f);
		Transform()->MarkLocalDirty();
	}
	if (Keyboard_IsKeyDown(KK_DOWN))
	{
		Transform()->Value().RotateAxis({ 1, 0, 0 }, 0.1f);
		Transform()->MarkLocalDirty();
	}


	if (Keyboard_IsKeyDownTrigger(KK_R))
	{
		SetApples();
	}
	if (Keyboard_IsKeyDownTrigger(KK_SPACE))
	{
		ShotApple();
	}

}

void CannonObject::SetApples()
{
	m_pApples = Manager::GetScene()->GetGameObjects<AppleObject>();
}

void CannonObject::ShotApple()
{
	// 位置をここに
	m_pApples[0]->Transform()->SetPosition(Transform()->Position());

	// 力を加えて飛ばす
	Vector3 vect = Transform()->Up() * 10;
	Rigidbody* rigid = m_pApples[0]->GetComponent<Rigidbody>();
	rigid->SetAngularVelocity(Vector3{});
	rigid->SetVelocity(Vector3{});
	rigid->ApplyImpulse(vect);

}

