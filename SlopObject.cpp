/*
	SlopObject.cpp
	20250913 hanaue sho
	リンゴのオブジェクト
*/
#include "SlopObject.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "SlopComponent.h"
#include "renderer.h"
#include "MathCommon.h"
#include "texture.h"  // Texture::Load 既存

#include "keyboard.h"
#include "Manager.h"
#include "Scene.h"
#include "CubeObject.h"

void SlopObject::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0, 7.5f, 14.5f });
	tf->SetScale({ 20.0f, 25.0f, 1.0f });
	tf->SetEulerAngles({ PI * 0.38f, 0, 0 });

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

	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\slop.png");
	// サンプラーは Renderer::Init() で 0番に PSSetSamplers 済みなら null でも描ける
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

	MATERIAL m{};
	m.Diffuse = XMFLOAT4(0.5f, 0.5f, 1, 1);
	m.Ambient = XMFLOAT4(1, 1, 1, 1);
	m.TextureEnable = TRUE;
	mat->SetMaterial(m);

	// 透明テクスチャの可能性が高いのでアルファブレンドに
	mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);

	// 4) MeshRenderer を追加（描画実行係）
	AddComponent<MeshRendererComponent>();


	// 物理を働かせたいのでコライダーなどを設定
	Collider* coll = AddComponent<Collider>();
	coll->SetBox({0.5f, 0.5f, 0.5f});
	coll->SetTrigger(false);

	Rigidbody* rigid = AddComponent<Rigidbody>();
	rigid->SetBodyType(Rigidbody::BodyType::Static);
	rigid->SetFrictionStatic(0.44f);
	rigid->SetFrictionDynamic(0.33f);

	AddComponent<SlopComponent>();

	// 障害物作るよー -----
	m_pObstancle[0] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[0]->Init();
	m_pObstancle[0]->Transform()->SetEulerAngles({ 1.2f, 0, 0 });
	m_pObstancle[0]->Transform()->SetScale({ 1, 25, 1 });
	m_pObstancle[0]->Transform()->SetPosition({ 10, 8.5f, 14.0f });

	m_pObstancle[1] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[1]->Init();
	m_pObstancle[1]->Transform()->SetEulerAngles({ 1.2f, 0, 0 });
	m_pObstancle[1]->Transform()->SetScale({ 1, 25, 1 });
	m_pObstancle[1]->Transform()->SetPosition({ -10, 8.5f, 14.0f });

	m_pObstancle[2] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[2]->Init();
	m_pObstancle[2]->Transform()->SetEulerAngles({ 1.2f, 0, 1 });
	m_pObstancle[2]->Transform()->SetScale({ 2, 0.5f, 0.4f });
	m_pObstancle[2]->Transform()->SetPosition({ -1.0f, 7.5f + -3 * sinf(0.37f), 12.0f + -3 * cosf(0.37f)});

	m_pObstancle[3] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[3]->Init();
	m_pObstancle[3]->Transform()->SetEulerAngles({ 1.2f, 0, -1 });
	m_pObstancle[3]->Transform()->SetScale({ 2, 0.5f, 0.4f });
	m_pObstancle[3]->Transform()->SetPosition({ 1.0f, 7.5f + -3 * sinf(0.37f), 12.0f + -3 * cosf(0.37f)});

	m_pObstancle[4] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[4]->Init();
	m_pObstancle[4]->Transform()->SetEulerAngles({ 1.2f, 0, 1 });
	m_pObstancle[4]->Transform()->SetScale({ 2, 0.5f, 0.4f });
	m_pObstancle[4]->Transform()->SetPosition({ 5, 7.5f + 6 * sinf(0.37f), 12.0f + 6 * cosf(0.37f)});

	m_pObstancle[5] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[5]->Init();
	m_pObstancle[5]->Transform()->SetEulerAngles({ 1.2f, 0, -1 });
	m_pObstancle[5]->Transform()->SetScale({ 2, 0.5f, 0.4f });
	m_pObstancle[5]->Transform()->SetPosition({ 7, 7.5f + 6 * sinf(0.37f), 12.0f + 6 * cosf(0.37f)});

	m_pObstancle[6] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[6]->Init();
	m_pObstancle[6]->Transform()->SetEulerAngles({ 1.2f, 0, 1 });
	m_pObstancle[6]->Transform()->SetScale({ 2, 0.5f, 0.4f });
	m_pObstancle[6]->Transform()->SetPosition({ -6, 7.5f + 8 * sinf(0.37f), 12.0f + 8 * cosf(0.37f)});

	m_pObstancle[7] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[7]->Init();
	m_pObstancle[7]->Transform()->SetEulerAngles({ 1.2f, 0, -1 });
	m_pObstancle[7]->Transform()->SetScale({ 2, 0.5f, 0.4f });
	m_pObstancle[7]->Transform()->SetPosition({ -4, 7.5f + 8 * sinf(0.37f), 12.0f + 8 * cosf(0.37f)});

	m_pObstancle[8] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[8]->Init();
	m_pObstancle[8]->Transform()->SetEulerAngles({ 1.2f, 0, 1 });
	m_pObstancle[8]->Transform()->SetScale({ 5.5f, 0.5f, 0.4f });
	m_pObstancle[8]->Transform()->SetPosition({ 8, 7.5f + 2 * sinf(0.37f), 12.0f + 2 * cosf(0.37f) });

	m_pObstancle[9] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[9]->Init();
	m_pObstancle[9]->Transform()->SetEulerAngles({ 1.2f, 0, -1 });
	m_pObstancle[9]->Transform()->SetScale({ 5.5f, 0.5f, 0.4f });
	m_pObstancle[9]->Transform()->SetPosition({ -8, 7.5f + 2 * sinf(0.37f), 12.0f + 2 * cosf(0.37f) });

	m_pObstancle[10] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[10]->Init();
	m_pObstancle[10]->Transform()->SetEulerAngles({ 0.0f, 0, 1 });
	m_pObstancle[10]->Transform()->SetScale({ 1.5f, 1.5f, 1.5f });
	m_pObstancle[10]->Transform()->SetPosition({ -5, 7.5f + 5 * sinf(0.37f), 12.0f + 5 * cosf(0.37f) });
	rigid = m_pObstancle[10]->GetComponent<Rigidbody>();
	rigid->SetBodyType(Rigidbody::BodyType::Dynamic);
	rigid->SetGravityMode(Rigidbody::GravityMode::Custom);
	rigid->SetCustomGravity({ 0,  -sinf(PI * 0.38f) * 10, cosf(PI * 0.38f) * 10 } );
	rigid->SetMass(6);
	rigid->ComputeBoxInertia({ 1.5f, 1.5f, 1.5f }, rigid->Mass());

	m_pObstancle[11] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pObstancle[11]->Init();
	m_pObstancle[11]->Transform()->SetEulerAngles({ 0.0f, 0, 1 });
	m_pObstancle[11]->Transform()->SetScale({ 1.5f, 1.5f, 1.5f });
	m_pObstancle[11]->Transform()->SetPosition({ 5, 7.5f + 5 * sinf(0.37f), 12.0f + 5 * cosf(0.37f) });
	rigid = m_pObstancle[11]->GetComponent<Rigidbody>();
	rigid->SetBodyType(Rigidbody::BodyType::Dynamic);
	rigid->SetGravityMode(Rigidbody::GravityMode::Custom);
	rigid->SetCustomGravity({ 0,  -sinf(PI * 0.38f) * 10, cosf(PI * 0.38f) * 10 });
	rigid->SetMass(6);
	rigid->ComputeBoxInertia({ 1.5f, 1.5f, 1.5f }, rigid->Mass());

	for (int i = 0; i < 12; i++)
	{
		m_pObstancle[i]->GetComponent<MaterialComponent>()->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
		m_pObstancle[i]->AddComponent<SlopComponent>();
	}

	m_Sign[0] = 1;
	m_Sign[1] = 1;
	m_Sign[2] = 1;
	m_Sign[3] = 1;
}

void SlopObject::Uninit()
{
	GameObject::Uninit();

	for (int i = 0; i < 12; i++)
		m_pObstancle[i]->RequestDestroy();
}

void SlopObject::Update(float dt)
{
	GameObject::Update(dt);

	// 2 ~ 9 の組み合わせの移動処理 -----
	float velo = 3 * dt;
	for (int i = 0; i < 3; i++)
	{
		Vector3 pos = m_pObstancle[(i + 1) * 2]->Transform()->Position();
		pos.x += velo * m_Sign[i];
		m_pObstancle[(i + 1) * 2]->Transform()->SetPosition(pos);
		pos = m_pObstancle[(i + 1) * 2 + 1]->Transform()->Position();
		pos.x += velo * m_Sign[i];
		m_pObstancle[(i + 1) * 2 + 1]->Transform()->SetPosition(pos);
		if (m_pObstancle[(i + 1) * 2]->Transform()->Position().x > 5 || m_pObstancle[(i + 1) * 2 + 1]->Transform()->Position().x < -5)
			m_Sign[i] *= -1;
	}

	// 10, 11 の回転体処理 -----
	Rigidbody* rigid10 = m_pObstancle[10]->GetComponent<Rigidbody>();
	rigid10->AddTorque({ 0, 150, 0 });
	Rigidbody* rigid11 = m_pObstancle[11]->GetComponent<Rigidbody>();
	rigid11->AddTorque({ 0, -150, 0 });


}

