/*
	PaperPolygone.cpp
	���ʂɃe�N�X�`����\���y���y���̃|���S��
	20241229 hanaue sho
*/
#include "PaperPolygon.h"
#include "renderer.h"
#include "texture.h"

static ID3D11Buffer* g_VertexBuffer = NULL; // ���_���
static ID3D11Buffer* g_IndexBuffer = NULL; // �C���f�b�N�X���
static ID3D11ShaderResourceView* g_pTextureId = nullptr; // �e�N�X�`��ID

PaperPolygon::PaperPolygon()
{
	ID3D11Device* pDevice = Renderer::GetDevice();

	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd = {}; // ZeroMemory �Ɠ���������
	bd.Usage = D3D11_USAGE_DEFAULT; // 
	bd.ByteWidth = sizeof(VERTEX_3D) * 8; // �o�b�t�@�T�C�Y�@�\���̃T�C�Y�~���_���� �P�ʓ�����O�p�`�Q��
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// ���_�z��
	VERTEX_3D vertex[8];

	// �\��
	vertex[0].Position = { -0.5f, 0.5f, 0.0f }; // ����
	vertex[0].TexCoord = { 0.0f, 0.0f };
	vertex[0].Normal = { 0.0f, 0.0f, -1.0f };
	vertex[0].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[1].Position = { 0.5f, 0.5f, 0.0f }; // �E��
	vertex[1].TexCoord = { 1.0f, 0.0f };
	vertex[1].Normal = { 0.0f, 0.0f, -1.0f };
	vertex[1].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[2].Position = { -0.5f, -0.5f, 0.0f }; // ����
	vertex[2].TexCoord = { 0.0f, 1.0f };
	vertex[2].Normal = { 0.0f, 0.0f, -1.0f };
	vertex[2].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[3].Position = { 0.5f, -0.5f, 0.0f }; // �E��
	vertex[3].TexCoord = { 1.0f, 1.0f };
	vertex[3].Normal = { 0.0f, 0.0f, -1.0f };
	vertex[3].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	// ����
	vertex[4].Position = { 0.5f, 0.5f, 0.0f }; // �E��
	vertex[4].TexCoord = { 0.0f, 0.0f };
	vertex[4].Normal = { 0.0f, 0.0f, 1.0f };
	vertex[4].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[5].Position = { -0.5f, 0.5f, 0.0f }; // ����
	vertex[5].TexCoord = { 1.0f, 0.0f };
	vertex[5].Normal = { 0.0f, 0.0f, 1.0f };
	vertex[5].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[6].Position = { 0.5f, -0.5f, 0.0f }; // �E��
	vertex[6].TexCoord = { 0.0f, 1.0f };
	vertex[6].Normal = { 0.0f, 0.0f, 1.0f };
	vertex[6].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertex[7].Position = { -0.5f, -0.5f, 0.0f }; // ����
	vertex[7].TexCoord = { 1.0f, 1.0f };
	vertex[7].Normal = { 0.0f, 0.0f, 1.0f };
	vertex[7].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

	// �C���f�b�N�X�z��
	unsigned short indexes[12] =
	{
		0, 1, 2,
		1, 3, 2,
		4, 5, 6,
		5, 7, 6
	};
	

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertex;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	Renderer::GetDevice()->CreateBuffer(&bd, &data, &g_VertexBuffer);

	// �C���f�b�N�X�o�b�t�@����
	// D3D11_BUFFER_DESC bd = {}; // ZeroMemory �Ɠ��������� �g���܂킷
	// bd.Usage = D3D11_USAGE_DEFAULT; // �g���܂킵�Ă�̂ŏ����Ȃ��Ă悢
	bd.ByteWidth = sizeof(unsigned short) * 12; // �o�b�t�@�T�C�Y�@�\���̃T�C�Y�~���_���� �P�ʓ�����O�p�`�Q��
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER; // �w��
	// bd.CPUAccessFlags = 0; // �g���܂킵�Ă�

	// D3D11_SUBRESOURCE_DATA data; // �g���܂킵
	data.pSysMem = indexes;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	Renderer::GetDevice()->CreateBuffer(&bd, &data, &g_IndexBuffer); // ���_�o�b�t�@�����

	// �e�N�X�`���ǂݍ���
	g_pTextureId = Texture::Load("asset\\texture\\whiteTexture.png");
}

PaperPolygon::~PaperPolygon()
{
	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
	if (g_IndexBuffer)
	{
		g_IndexBuffer->Release();
		g_IndexBuffer = NULL;
	}
}

void PaperPolygon::Draw() const
{
	// �e�N�X�`���̓ǂݍ���
	//ID3D11ShaderResourceView* srv = GetTexture(g_textureId);
	//GetDeviceContext()->PSSetShaderResources(0, 1, &srv);

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D); // ���_�̃T�C�Y
	UINT offset = 0; // ���_�o�b�t�@��̃X�^�[�g�̏��
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); // g_VertexBuffer �\�����钸�_�o�b�t�@�[�̃|�C���^�[

	// �C���f�b�N�X�o�b�t�@�ݒ�
	Renderer::GetDeviceContext()->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// �v���~�e�B�u�g�|���W�ݒ�
	if (!m_IsSkeleton)
		Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	else
		Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = GetColor();
	Renderer::SetMaterial(material);

	// �|���S���`��
	//GetDeviceContext()->Draw(8, 0); // Draw �̑�P�����͕\�����钸�_���̐�
	Renderer::GetDeviceContext()->DrawIndexed(12, 0, 0); // �C���f�b�N�X�ł̕`��
	//GetDeviceContext()->DrawIndexed(36, 0, 0); // �C���f�b�N�X�ł̕`��
}
