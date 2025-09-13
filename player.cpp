/*
	player.cpp
	20250514 hanaue sho
*/
#include "player.h"
#include "Transform.h"


void Player::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	tf->SetScale({ 1,1,1 });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	auto* mf = AddComponent<MeshFilterComponent>();
	//MeshFactory::CreateCube(mf, { {2.0f, 2.0f, 2.0f}});
	MeshFactory::CreateSphere(mf, { 1, 12, 12});
	//MeshFactory::CreateCylinder(mf, { 2, 20, 12});
	//MeshFactory::CreateCapsule(mf, { 2, 12, 12});

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
	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\apple.png");
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

	
	// 物理を働かせたいのでコライダーなどを設定
	Collider* coll = AddComponent<Collider>();
	coll->SetSphere(1);
	coll->SetTrigger(false);

	Rigidbody* rigid = AddComponent<Rigidbody>();
	rigid->SetGravityScale(1.0f);
}


void Player::Update(float dt)
{
	GameObject::Update(dt);

	//Transform()->Value().RotateAxis({0, 1, 0}, 0.01f);
	Transform()->MarkLocalDirty();

}
