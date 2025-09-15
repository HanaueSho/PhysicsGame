/*
	AppleObject.cpp
	20250913 hanaue sho
	�����S�̃I�u�W�F�N�g
*/
#include "AppleObject.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "AppleComponent.h"
#include "renderer.h"
#include "texture.h"  // Texture::Load ����


void AppleObject::Init()
{
	// 1) Transform�i���� GameObject ctor �Œǉ��ς݁j���擾���ď����p�������Ă���
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	tf->SetScale({ 1.0f, 1.0f, 1.0f });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter ��ǉ����Ē��_�o�b�t�@�i4���_�̋�`�j�����
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateApple(mf, { 1, 24, 24 });

	// 3) Material ��ǉ��i�V�F�[�_/�e�N�X�`��/�}�e���A���j
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\pixelLightingVS.cso");
	Renderer::CreatePixelShader(&ps, "shader\\pixelLightingPS.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\appleTexture.png");
	// �T���v���[�� Renderer::Init() �� 0�Ԃ� PSSetSamplers �ς݂Ȃ� null �ł��`����
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

	MATERIAL m{};
	m.Diffuse = XMFLOAT4(1, 1, 1, 1);
	m.Ambient = XMFLOAT4(1, 1, 1, 1);
	m.TextureEnable = TRUE;
	mat->SetMaterial(m);

	// �����e�N�X�`���̉\���������̂ŃA���t�@�u�����h��
	mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);

	// 4) MeshRenderer ��ǉ��i�`����s�W�j
	AddComponent<MeshRendererComponent>();


	// �����𓭂��������̂ŃR���C�_�[�Ȃǂ�ݒ�
	Collider* coll = AddComponent<Collider>();
	coll->SetSphere(1);
	coll->SetTrigger(false);

	Rigidbody* rigid = AddComponent<Rigidbody>();
	rigid->SetGravityScale(1.0f);
	rigid->SetMass(0.4f);
	rigid->SetFrictionDynamic(0.15f);
	rigid->SetFrictionStatic(0.4f);
	rigid->ComputeSphereInertia(tf->Scale().x, rigid->Mass());

	auto* ac = AddComponent<AppleComponent>();
	ac->Init();
}

void AppleObject::Update(float dt)
{
	GameObject::Update(dt);

	if (Transform()->Position().y < -3)
		RequestDestroy();
}
