/*
	player.cpp
	20250514 hanaue sho
*/
#include "player.h"
#include "Transform.h"


void Player::Init()
{
	// 1) Transform�i���� GameObject ctor �Œǉ��ς݁j���擾���ď����p�������Ă���
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	tf->SetScale({ 1,1,1 });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter ��ǉ����Ē��_�o�b�t�@�i4���_�̋�`�j�����
	auto* mf = AddComponent<MeshFilterComponent>();
	//MeshFactory::CreateCube(mf, { {2.0f, 2.0f, 2.0f}});
	MeshFactory::CreateSphere(mf, { 1, 12, 12});
	//MeshFactory::CreateCylinder(mf, { 2, 20, 12});
	//MeshFactory::CreateCapsule(mf, { 2, 12, 12});

	// 3) Material ��ǉ��i�V�F�[�_/�e�N�X�`��/�}�e���A���j
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	//Renderer::CreateVertexShader(&vs, &il, "shader\\unlitTextureVS.cso");
	//Renderer::CreatePixelShader(&ps, "shader\\unlitTexturePS.cso");
	Renderer::CreateVertexShader(&vs, &il, "shader\\pixelLightingVS.cso");
	Renderer::CreatePixelShader(&ps, "shader\\pixelLightingPS.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	// �� Polygon2D �Ɠ��� kirby ���g��
	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\apple.png");
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
}


void Player::Update(float dt)
{
	GameObject::Update(dt);

	//Transform()->Value().RotateAxis({0, 1, 0}, 0.01f);
	Transform()->MarkLocalDirty();

}
