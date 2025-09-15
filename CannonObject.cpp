/*
	CannonObject.cpp
	20250914 hanaue sho
	��C�̃I�u�W�F�N�g
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
#include "texture.h"  // Texture::Load ����

#include "Keyboard.h"
#include "Manager.h"
#include "Scene.h"
#include "AppleObject.h"
#include "Camera.h"
#include "Audio.h"

void CannonObject::Init()
{
	// 1) Transform�i���� GameObject ctor �Œǉ��ς݁j���擾���ď����p�������Ă���
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0, 4.5f, -16.2 });
	tf->SetScale({ 1.0f, 1.0f, 1.0f });
	tf->SetEulerAngles({ PI / 4.0f, 0, 0 });

	// 2) MeshFilter ��ǉ����Ē��_�o�b�t�@�i4���_�̋�`�j�����
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateCylinder(mf, { 0.6f, 2, 24, 6 });

	// 3) Material ��ǉ��i�V�F�[�_/�e�N�X�`��/�}�e���A���j
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\pixelLightingVS.cso");
	Renderer::CreatePixelShader(&ps, "shader\\pixelLightingPS.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\cannon.png");
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


	// SE
	m_pSE = new Audio();
	m_pSE->Load("assets\\audio\\cannon.wav");
}

void CannonObject::Update(float dt)
{
	GameObject::Update(dt);

	// �p�x���� -----
	const float valueAng = 0.01f;
	if (Keyboard_IsKeyDown(KK_UP))
	{
		Transform()->Value().RotateAxis({ 1, 0, 0 }, -valueAng);
		Transform()->MarkLocalDirty();
	}
	if (Keyboard_IsKeyDown(KK_DOWN))
	{
		Transform()->Value().RotateAxis({ 1, 0, 0 }, valueAng);
		Transform()->MarkLocalDirty();
	}
	if (Transform()->EulerAngles().x > MAX_ANGLE)
	{
		Vector3 angle = Transform()->EulerAngles();
		angle.x = MAX_ANGLE;
		Transform()->SetEulerAngles(angle);
	}
	if (Transform()->EulerAngles().x < MIN_ANGLE)
	{
		Vector3 angle = Transform()->EulerAngles();
		angle.x = MIN_ANGLE;
		Transform()->SetEulerAngles(angle);
	}

	// �ړ����� -----
	if (!m_IsMove) return;
	float valueMove = 0.03f * dt;

	if (Keyboard_IsKeyDown(KK_LEFT))
	{
		if (m_Acceleration.x > 0) m_Acceleration.x = 0;
		m_Acceleration.x -= valueMove;
	}
	else if (Keyboard_IsKeyDown(KK_RIGHT))
	{
		if (m_Acceleration.x < 0) m_Acceleration.x = 0;
		m_Acceleration.x += valueMove;
	}
	else
	{
		m_Acceleration.x = 0;
	}

	m_Velocity.x *= 0.9f; // ����
	m_Velocity += m_Acceleration;
	if (m_Acceleration.x > MAX_ACCE) m_Acceleration.x = MAX_ACCE;
	if (m_Acceleration.x < -MAX_ACCE) m_Acceleration.x = -MAX_ACCE;
	if (m_Velocity.x > MAX_VELOCITY) m_Velocity.x = MAX_VELOCITY;
	if (m_Velocity.x < -MAX_VELOCITY) m_Velocity.x = -MAX_VELOCITY;

	// �����𓮂���
	Transform()->SetPosition(Transform()->Position() + m_Velocity);
	if (Transform()->Position().x > 10)
	{
		Transform()->SetPosition({ 10, Transform()->Position().y, Transform()->Position().z });
		m_Velocity.x = 0;
		m_Acceleration.x = 0;
	}
	else if (Transform()->Position().x < -10)
	{
		Transform()->SetPosition({ -10, Transform()->Position().y, Transform()->Position().z });
		m_Velocity.x = 0;
		m_Acceleration.x = 0;
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
	if (!m_IsShot) return;
	if (CheckEmptyApples()) return;

	// �ʒu��������
	m_pApples[0]->Transform()->SetPosition(Transform()->Position());

	// �͂������Ĕ�΂�
	Vector3 vect = Transform()->Up() * 10;
	Rigidbody* rigid = m_pApples[0]->GetComponent<Rigidbody>();
	rigid->SetAngularVelocity(Vector3{});
	rigid->SetVelocity(Vector3{});
	rigid->ApplyImpulse(vect);

	// �z�񂩂�폜
	m_pApples.erase(m_pApples.begin());

	m_pSE->Play(false);
}

