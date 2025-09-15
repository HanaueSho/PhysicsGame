/*
	GroundObject.cpp
	20250914 hanaue sho
	�n�ʂ̃I�u�W�F�N�g
*/
#include "GroundObject.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "renderer.h"
#include "texture.h"  // Texture::Load ����

#include "GroundComponent.h"


void GroundObject::Init()
{
	// 1) Transform�i���� GameObject ctor �Œǉ��ς݁j���擾���ď����p�������Ă���
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0, -1.0f, 10.0f });
	tf->SetScale({ 1000.0f, 1.0f, 1000.0f });
	tf->SetEulerAngles({ 0, 0, 0 });

	// 2) MeshFilter ��ǉ����Ē��_�o�b�t�@�i4���_�̋�`�j�����
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreatePlane(mf, { 1, 1});

	// 3) Material ��ǉ��i�V�F�[�_/�e�N�X�`��/�}�e���A���j
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\pixelLightingVS.cso");
	Renderer::CreatePixelShader(&ps, "shader\\pixelLightingPS.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\ground.png");
	// �T���v���[�� Renderer::Init() �� 0�Ԃ� PSSetSamplers �ς݂Ȃ� null �ł��`����
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

	MATERIAL m{};
	m.Diffuse = XMFLOAT4(0.5f, 0.5f, 1, 1);
	m.Ambient = XMFLOAT4(1, 1, 1, 1);
	m.TextureEnable = TRUE;
	mat->SetMaterial(m);

	// �����e�N�X�`���̉\���������̂ŃA���t�@�u�����h��
	mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);

	// 4) MeshRenderer ��ǉ��i�`����s�W�j
	AddComponent<MeshRendererComponent>();

	// �����𓭂��������̂ŃR���C�_�[�Ȃǂ�ݒ�
	Collider* coll = AddComponent<Collider>();
	coll->SetBox({1, 1, 1});
	coll->SetTrigger(true);

	Rigidbody* rigid = AddComponent<Rigidbody>();
	rigid->SetBodyType(Rigidbody::BodyType::Kinematic);
	rigid->SetMass(1.0f);

	// �R���|�[�l���g
	AddComponent<GroundComponent>(); 


}

void GroundObject::Update(float dt)
{
	GameObject::Update(dt);

}
