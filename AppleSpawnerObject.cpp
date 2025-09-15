/*
	AppleSpawnerObject.cpp
	20250913 hanaue sho
	�����S�̃I�u�W�F�N�g
*/
#include "AppleSpawnerObject.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "renderer.h"
#include "texture.h"  // Texture::Load ����

#include "Manager.h"
#include "Scene.h"
#include "AppleObject.h"
#include "Random.h"


void AppleSpawnerObject::Init()
{
	// 1) Transform�i���� GameObject ctor �Œǉ��ς݁j���擾���ď����p�������Ă���
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0, 20, 21 });
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

	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\kirby.png");
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


}

void AppleSpawnerObject::Update(float dt)
{
	GameObject::Update(dt);

	if (m_TimerSum > 55) return;
	// �������� -----
	m_Timer += dt;
	m_TimerSum += dt;
	if (m_Timer > m_Interval)
	{
		float rand = Random::RandomRange(0.8f, 1.0f);
		float scale = 0.4f * rand;
		AppleObject* apple = Manager::GetScene()->AddGameObject<AppleObject>(1);
		apple->Init();
		apple->Transform()->SetScale({ scale, scale, scale });
		apple->Transform()->SetPosition(Transform()->Position());
		Rigidbody* rigid = apple->GetComponent<Rigidbody>();
		rigid->AddTorque({ 200, 0, 0 });
		rigid->SetAngDamping(0.1f);
		rigid->SetLinDamping(0.05f);
		rigid->SetFrictionDynamic(0.15f);
		rigid->SetFrictionStatic(0.2f);
		rigid->ComputeSphereInertia(apple->Transform()->Scale().x, rigid->Mass());
		m_Timer = 0.0f;

		m_Interval = 1.0f - 0.6f * (60.0f - m_TimerSum) / 60.0f;
	}

	// �ړ����� -----
	float rand = Random::RandomRange(0, 6);
	Vector3 pos = Transform()->Position();
	pos.x += (5 + rand * rand) * dt * m_Sign;
	Transform()->SetPosition(pos);

	if (Transform()->Position().x > 9)
	{
		Vector3 pos = Transform()->Position();
		pos.x = 9;
		Transform()->SetPosition(pos);
		m_Sign = -1;
	}
	if (Transform()->Position().x < -9)
	{
		Vector3 pos = Transform()->Position();
		pos.x = -9;
		Transform()->SetPosition(pos);
		m_Sign = 1;
	}
}
