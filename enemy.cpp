/*
	enemy.cpp
	20250514 hanaue sho
*/
#include "enemy.h"
#include "Collision.h"
#include "Keyboard.h"


void Enemy::Init()
{
	// 1) Transform�i���� GameObject ctor �Œǉ��ς݁j���擾���ď����p�������Ă���
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	tf->SetScale({ 1,1,1 });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter ��ǉ����Ē��_�o�b�t�@�i4���_�̋�`�j�����
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateCube(mf, { {2.0f, 2.0f, 2.0f}});
	//MeshFactory::CreateSphere(mf, { 1, 12, 12});
	//MeshFactory::CreateCylinder(mf, { 2, 20, 12 });
	//MeshFactory::CreateCapsule(mf, { 1, 12, 12});

	// 3) Material ��ǉ��i�V�F�[�_/�e�N�X�`��/�}�e���A���j
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
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

	// �����蔻��ǉ�
	Collider* coll = AddComponent<Collider>();
	coll->SetBox({1, 1, 1});
	coll->SetTrigger(false);

	// Rigidbody
	Rigidbody* rigid = AddComponent<Rigidbody>();
	rigid->ComputeBoxInertia({ 1, 1, 1 }, 1);
	//rigid->SetBodyType(Rigidbody::BodyType::Kinematic);
	rigid->AddForce({ 0, 500, 0 });
	rigid->SetMass(1);
}

void Enemy::Update(float dt)
{
	GameObject::Update(dt);

	float weight = 2.0f;
	float value = 0.1f;
	Vector3 vect{};
	GetComponent<Rigidbody>()->AddTorque({ 0, 10.0f, 0});

	if (Keyboard_IsKeyDownTrigger(KK_SPACE))
	{
		GetComponent<Rigidbody>()->AddForce({0, 200 * weight, 0});
		vect.y += value;
	}
	if (Keyboard_IsKeyDownTrigger(KK_LEFTSHIFT))
	{
		GetComponent<Rigidbody>()->AddForce({ 0, 200 * weight, 0 });
		vect.y -= value;
	}
	if (Keyboard_IsKeyDownTrigger(KK_LEFT))
	{
		GetComponent<Rigidbody>()->AddForce({ -300 * weight, 0, 0});
		vect.x -= value;
	}
	if (Keyboard_IsKeyDownTrigger(KK_RIGHT))
	{
		GetComponent<Rigidbody>()->AddForce({ 300 * weight, 0, 0 });
		vect.x += value;
	}
	if (Keyboard_IsKeyDownTrigger(KK_UP))
	{
		GetComponent<Rigidbody>()->AddForce({ 0, 0, 300 * weight });
		vect.z += value;
	}
	if (Keyboard_IsKeyDownTrigger(KK_DOWN))
	{
		GetComponent<Rigidbody>()->AddForce({ 0, 0, -300 * weight });
		vect.z -= value;
	}
	//Transform()->SetPosition(Transform()->Position() + vect);

	if (Keyboard_IsKeyDownTrigger(KK_D1))
		GetComponent<Rigidbody>()->AddTorque({ 0.0f, 0, 100 });
	if (Keyboard_IsKeyDownTrigger(KK_D2))
		GetComponent<Rigidbody>()->AddTorque({ 0.0f, 0, -100 });
}

