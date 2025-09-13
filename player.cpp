/*
	player.cpp
	20250514 hanaue sho
*/
#include "player.h"
#include "Transform.h"
#include "Manager.h"
#include "Keyboard.h"
#include "CubeObject.h"
#include "MeshRendererComponent.h"


void Player::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	tf->SetScale({ 1.0f, 1.0f, 1.0f });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	auto* mf = AddComponent<MeshFilterComponent>();
	//MeshFactory::CreateCube(mf, { {2.0f, 2.0f, 2.0f}});
	//MeshFactory::CreateSphere(mf, { 1, 12, 12});
	//MeshFactory::CreateCylinder(mf, { 2, 20, 12});
	//MeshFactory::CreateCapsule(mf, { 2, 12, 12});
	MeshFactory::CreateApple(mf, { 1, 24, 24 });

	// 3) Material を追加（シェーダ/テクスチャ/マテリアル）
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	//Renderer::CreateVertexShader(&vs, &il, "shader\\unlitTextureVS.cso");
	//Renderer::CreatePixelShader(&ps, "shader\\unlitTexturePS.cso");
	Renderer::CreateVertexShader(&vs, &il, "shader\\pixelLightingVS.cso");
	Renderer::CreatePixelShader(&ps, "shader\\pixelLightingPS.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	// 旧 Polygon2D と同じ kirby を使う
	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\appleTexture.png");
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



	// 箱の形状の設定 -----
	Vector3 size = { 10, 10, 10 };
	float thick = 1.0f;

	m_pCubes[0] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pCubes[0]->Init();
	m_pCubes[0]->Transform()->SetScale({ size.x + thick * 2, thick, size.z + thick * 2 }); // 底面
	m_pCubes[0]->Transform()->SetPosition({ 0, 0, 0 }); // 底面

	m_pCubes[1] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pCubes[1]->Init();
	m_pCubes[1]->Transform()->SetScale({ thick, size.y, size.z + thick * 2 }); // 東
	m_pCubes[1]->Transform()->SetPosition({ size.x * 0.5f + thick * 0.5f, size.y * 0.5f + thick * 0.5f, 0 }); // 東

	m_pCubes[2] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pCubes[2]->Init();
	m_pCubes[2]->Transform()->SetScale({ thick, size.y, size.z + thick * 2 }); // 西
	m_pCubes[2]->Transform()->SetPosition({ -size.x * 0.5f - thick * 0.5f, size.y * 0.5f + thick * 0.5f, 0 }); // 西

	m_pCubes[3] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pCubes[3]->Init();
	m_pCubes[3]->Transform()->SetScale({ size.x + thick * 2.0f, size.y, thick }); // 北
	m_pCubes[3]->Transform()->SetPosition({ 0, size.y * 0.5f + thick * 0.5f, size.z * 0.5f + thick * 0.5f }); // 北

	m_pCubes[4] = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pCubes[4]->Init();
	m_pCubes[4]->Transform()->SetScale({ size.x + thick * 2.0f, size.y, thick }); // 南
	m_pCubes[4]->Transform()->SetPosition({ 0, size.y * 0.5f + thick * 0.5f, -size.z * 0.5f - thick * 0.5f }); // 南
	m_pCubes[4]->GetComponent<MeshRendererComponent>()->SetDraw(false);

	// 最後に場所を指定
	Vector3 worldPosition{ 0, -10, -15 };
	tf->SetPosition(worldPosition);
	for (int i = 0; i < 5; i++)
		m_pCubes[i]->Transform()->SetPosition(m_pCubes[i]->Transform()->Position() + worldPosition);



}


void Player::Update(float dt)
{
	GameObject::Update(dt);

	// 移動処理 -----
	float value = 0.1f;
	Vector3 vect{};

	if (Keyboard_IsKeyDown(KK_SPACE))
	{
		vect.y += value;
	}
	if (Keyboard_IsKeyDown(KK_LEFTSHIFT))
	{
		vect.y -= value;
	}
	if (Keyboard_IsKeyDown(KK_LEFT))
	{
		vect.x -= value;
	}
	if (Keyboard_IsKeyDown(KK_RIGHT))
	{
		vect.x += value;
	}
	if (Keyboard_IsKeyDown(KK_UP))
	{
		vect.z += value;
	}
	if (Keyboard_IsKeyDown(KK_DOWN))
	{
		vect.z -= value;
	}

	// 自分を動かす
	Transform()->SetPosition(Transform()->Position() + vect);

	// 箱も動かす
	for (int i = 0; i < 5; i++)
		m_pCubes[i]->Transform()->SetPosition(m_pCubes[i]->Transform()->Position() + vect);

}
