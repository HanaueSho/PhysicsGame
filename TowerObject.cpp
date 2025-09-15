/*
	TowerObject.cpp
	20250914 hanaue sho
	�^���[�̃I�u�W�F�N�g
*/
#include "TowerObject.h"
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
#include "CubeObject.h"
#include "BlockComponent.h"

void TowerObject::Init()
{
	// 1) Transform�i���� GameObject ctor �Œǉ��ς݁j���擾���ď����p�������Ă���
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0, 0, 0 });
	tf->SetScale({ 1.0f, 1.0f, 1.0f });
	tf->SetEulerAngles({ 0, 0, 0 });

	// 2) MeshFilter ��ǉ����Ē��_�o�b�t�@�i4���_�̋�`�j�����
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateCube(mf, { {1, 1, 1} });

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
	//AddComponent<MeshRendererComponent>();
}

void TowerObject::Update(float dt)
{
	GameObject::Update(dt);

}

void TowerObject::CreateTable(const Vector3& worldPosition)
{
	Vector3 sizeTable = { 7, 0.5f, 2.5f };
	// �e�[�u������
	m_pTable = Manager::GetScene()->AddGameObject<CubeObject>(1);
	m_pTable->Init();
	m_pTable->Transform()->SetScale(sizeTable);
	m_pTable->Transform()->SetPosition(worldPosition);
	m_pTable->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static);

	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\cannon.png");
	m_pTable->GetComponent<MaterialComponent>()->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
}

void TowerObject::CreateTower(int index, const Vector3& offset)
{
	Vector3 size = m_pTable->Transform()->Scale();
	Vector3 sizeBlock = { 1, 1, 1 };
	int num = 0;

	// �^���[����
	for (int i = index; i < index + 3; i++)
	{
		float weight = (float)pow(0.75f, num);
		m_pBlocks[i] = Manager::GetScene()->AddGameObject<CubeObject>(1);
		m_pBlocks[i]->Init();
		m_pBlocks[i]->Transform()->SetScale({ sizeBlock.x * weight, sizeBlock.y * weight, sizeBlock.z * weight });
		m_pBlocks[i]->Transform()->SetPosition({ 0, 0.0f + size.y * 0.5f + sizeBlock.y  * (num + 0.5f), 0 });
		m_pBlocks[i]->AddComponent<BlockComponent>();
		Rigidbody* rigid = m_pBlocks[i]->GetComponent<Rigidbody>();
		rigid->SetBodyType(Rigidbody::BodyType::Dynamic);
		rigid->SetGravityScale(1.0f);
		rigid->SetMass(1.0f * weight);
		rigid->ComputeBoxInertia(m_pBlocks[i]->Transform()->Scale(), rigid->Mass());

		num++;
	}

	// �Ō�ɏꏊ���w��
	Vector3 worldPosition = m_pTable->Transform()->Position() + offset;
	for (int i = index; i < index + 3; i++)
		m_pBlocks[i]->Transform()->SetPosition(m_pBlocks[i]->Transform()->Position() + worldPosition);

	// �e�N�X�`�����f
	auto* srv = Texture::Load("assets\\texture\\block.png");
	for (int i = index; i < index + 3; i++)
	{
		auto* mat = m_pBlocks[i]->GetComponent<MaterialComponent>();
		mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
	}

}

void TowerObject::CreateBridge()
{
}

bool TowerObject::CheckBlocks()
{
	bool b = true;

	for (int i = 0; i < 9; i++)
	{
		if (!m_pBlocks[i]) continue;

		if (!m_pBlocks[i]->GetComponent<BlockComponent>()->IsCounted()) // ��ł��܂� false
			return false;
	}

	return b;
}

