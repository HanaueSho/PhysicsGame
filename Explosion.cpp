/*
	explosion.cpp
	20250611 hanaue sho
*/
#include "explosion.h"
#include "main.h"
#include "renderer.h"
#include "Manager.h"
#include "camera.h"
#include "texture.h"
#include "scene.h"

void Explosion::Init()
{
	VERTEX_3D vertex[4];

	vertex[0].Position = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(1.0f, 1.0f, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(1.0f, -1.0f, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC; // ���_�f�[�^��G��̂� DYNAMIC �ɂ��� ----------
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // map �Œ��_�f�[�^��������������悤�ɂ��� ----------

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertex;

	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_pVertexBuffer);

	// �e�N�X�`���ǂݍ���
	//TexMetadata metadata;
	//ScratchImage image;
	//LoadFromWICFile(L"assets\\texture\\explosion.png", WIC_FLAGS_NONE, &metadata, image);
	////LoadFromWICFile(L"assets\\texture\\HumanTube.png", WIC_FLAGS_NONE, &metadata, image);
	//CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &m_pTexture);
	//assert(m_pTexture);

	m_pTexture = Texture::Load("assets\\texture\\explosion.png");

	Renderer::CreateVertexShader(&m_pVertexShader, &m_pVertexLayout, "shader\\unlitTextureVS.cso");

	Renderer::CreatePixelShader(&m_pPixelShader, "shader\\unlitTexturePS.cso");

	// ����������
	m_Frame = 0;

}

void Explosion::Uninit()
{
	m_pVertexBuffer->Release();
	m_pVertexShader->Release();
	m_pPixelShader->Release();
	m_pVertexLayout->Release();
	//m_pTexture->Release();
}

void Explosion::Update()
{
	/*
	m_Frame++;
	if (m_Frame > 15) SetDestroy();
	*/
}

void Explosion::Draw()
{
	/*
	// ���_�f�[�^��������
	D3D11_MAPPED_SUBRESOURCE msr;
	Renderer::GetDeviceContext()->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float textureWidth  = 1.0f / 4; // �e�N�X�`���̉��̐�
	float textureHeight = 1.0f / 4; // �e�N�X�`���̏c�̐�
	float textureX = (m_Frame % 4) * textureWidth; // �e�N�X�`���̉����W
	float textureY = (m_Frame / 4) * textureHeight; // �e�N�X�`���̏c���W

	vertex[0].Position = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(textureX, textureY);

	vertex[1].Position = XMFLOAT3(1.0f, 1.0f, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(textureX + textureWidth, textureY);

	vertex[2].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(textureX, textureY + textureHeight);

	vertex[3].Position = XMFLOAT3(1.0f, -1.0f, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(textureX + textureWidth, textureY + textureHeight);

	Renderer::GetDeviceContext()->Unmap(m_pVertexBuffer, 0);


	// ���̓��C�A�E�g
	Renderer::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

	// �V�F�[�_�[�ݒ�
	Renderer::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

	// �}�g���N�X�ݒ�
	XMMATRIX world, scale, rotate, translation;
	scale = XMMatrixScaling(m_Transform.scale.x, m_Transform.scale.y, m_Transform.scale.z);
	rotate = XMMatrixRotationRollPitchYaw(m_Transform.rotation.ToEulerAngles().x, m_Transform.rotation.ToEulerAngles().y, m_Transform.rotation.ToEulerAngles().z);
	translation = XMMatrixTranslation(m_Transform.position.x, m_Transform.position.y, m_Transform.position.z);

	// �r���{�[�h�}�g���N�X
	XMMATRIX viewMtxInverse = XMMatrixTranspose(Manager::GetScene()->GetGameObject<Camera>()->GetMatrixView()); // �t�s���]�u�ϊ��ɂ�蓾��
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, viewMtxInverse);
	matrix._14 = matrix._24 = matrix._34 = 0.0f; // ���s�ړ��������J�b�g�i�]�u����Ă�̂ł����ɂȂ�j
	XMMATRIX billboardMatrix = XMLoadFloat4x4(&matrix);

	world = scale * rotate * billboardMatrix * translation;
	Renderer::SetWorldMatrix(world);

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// �e�N�X�`���ݒ�
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_pTexture);

	// �}�e���A���ݒ�
	MATERIAL material{};
	material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	// �v���~�e�B�u�g�|���W�ݒ�
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �|���S���`��
	Renderer::GetDeviceContext()->Draw(4, 0);
	*/
}
