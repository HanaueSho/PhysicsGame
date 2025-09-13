/*
	SpherePolygone.cpp
	20241212 hanaue sho
*/
#include "SpherePolygon.h"
#include "renderer.h"
#include "texture.h"


static ID3D11Buffer* g_VertexBuffer = NULL; // ���_���
static ID3D11Buffer* g_IndexBuffer = NULL; // �C���f�b�N�X���
static ID3D11ShaderResourceView* g_pTextureId = nullptr; // �e�N�X�`��ID


SpherePolygon::SpherePolygon(int division, bool isReverse)
{
	m_IsReverse = isReverse;
	// ���_���̌v�Z
	const int divisionH = 12; // ��������������
	const int divisionV = 12; // ��������������
	const int numVertex = divisionV * (divisionH + 1); // �����_��
	const int numIndex = (divisionH * 2 + divisionH * (divisionV - 2) * 2) * 3; // �C���f�b�N�X��
	XMFLOAT3 radian = {}; // �p�x
	
	m_DivisionH = divisionH; // �����o�ϐ��Ɋi�[
	m_DivisionV = divisionV; // �����o�ϐ��Ɋi�[


	ID3D11Device* pDevice = Renderer::GetDevice();

	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd = {}; // ZeroMemory �Ɠ���������
	bd.Usage = D3D11_USAGE_DEFAULT; // 
	bd.ByteWidth = sizeof(VERTEX_3D) * numVertex; // �o�b�t�@�T�C�Y�@�\���̃T�C�Y�~���_���� �P�ʓ�����O�p�`�Q��
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// ���_�z��
	VERTEX_3D vertex[numVertex] = {};

	// �C���f�b�N�X�z��
	unsigned short indexes[numIndex] = {};
	int count = 0; // �J�E���^�[
	
	for (int i = 0; i < divisionH + 1; i++) // ������������
	{
		radian.y = (180.0f / divisionH) * i * XM_PI / 180; // �����W�̊p�x�X�V

		for (int j = 0; j < divisionV; j++) // ������������
		{
			// �p�x�X�V -----
			radian.x = (360.0f / divisionV) * j * XM_PI / 180;
			radian.z = (360.0f / divisionV) * j * XM_PI / 180;

			// ���_�̍��W�v�Z -----
			XMFLOAT3 position = {};
			position.y = cosf(radian.y) * 0.5f;
			position.x = sinf(radian.x) * sinf(radian.y) * 0.5f;
			position.z = cosf(radian.z) * sinf(radian.y) * 0.5f;

			vertex[i * divisionV + j].Position = position; // ���W�i�[

			// �e�b�N�X�R�[�h�̐ݒ� -----
			if (i % 2 == 0 && j % 2 == 0)
			{
				vertex[i * divisionV + j].TexCoord = {0.0f, 0.0f};
			}
			else if (i % 2 == 0 && j % 2 == 1)
			{
				vertex[i * divisionV + j].TexCoord = { 1.0f, 0.0f };
			}
			else if (i % 2 == 1 && j % 2 == 0)
			{
				vertex[i * divisionV + j].TexCoord = { 0.0f, 1.0f};
			}
			else if (i % 2 == 1 && j % 2 == 1)
			{
				vertex[i * divisionV + j].TexCoord = { 1.0f, 1.0f };
			}

			// �@���x�N�g���̐ݒ� -----
			vertex[i * divisionV + j].Normal = position;
			 
			// �F�̐ݒ� -----
			vertex[i * divisionV + j].Diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
			
			// ������ index ���������� -----
			//if (i != 0) // �����v��Ȃ�
			{
				if (i > 1 && i < divisionH) // ��ԏ�ƈ�ԉ��͏������قȂ�
				{
					indexes[count] = (i - 1) * divisionV + j; // ��
					indexes[count + 1] = i * divisionV + j; // ��������
					indexes[count + 2] = i * divisionV + j + 1; // �ƂȂ�

					indexes[count + 3] = (i - 1) * divisionV + j; // ��
					indexes[count + 4] = i * divisionV + j + 1; // �ƂȂ�
					indexes[count + 5] = (i - 1) * divisionV + j + 1; // ��ƂȂ�

					if (j == divisionV - 1) // �Ō�̎O�p�`��␳
					{
						indexes[count + 2] = i * divisionV + j + 1 - divisionV; // �ƂȂ�
						
						indexes[count + 4] = i * divisionV + j + 1 - divisionV; // �ƂȂ�
						indexes[count + 5] = (i - 1) * divisionV + j + 1 - divisionV; // ��ƂȂ�
					}

					count += 6;
				}
				else if (i == 1) // ��ԏ�
				{
					indexes[count] = (i - 1) * divisionV + j; // ��
					indexes[count + 1] = i * divisionV + j; // ��������
					indexes[count + 2] = i * divisionV + j + 1; // �ƂȂ�
					
					if (j == divisionH - 1) // �Ō�̎O�p�`��␳
					{
						indexes[count + 2] = i * divisionV + j + 1 - divisionV; // �ƂȂ�
					}

					count += 3;
				}
				else if (i == divisionH) // ��ԉ�
				{
					indexes[count] = (i - 1) * divisionV + j; // ��
					indexes[count + 1] = i * divisionV + j; // ��������
					indexes[count + 2] = (i - 1) * divisionV + j + 1; // ��ƂȂ�

					if (j == divisionV - 1) // �Ō�̎O�p�`��␳
					{
						indexes[count + 2] = (i - 1) * divisionV + j + 1 - divisionV; // ��ƂȂ�
					}

					count += 3;
				}
			}
		}
	}



	if (m_IsReverse)
	{
		unsigned short dummy = 0;
		for (int i = 0; i < numIndex; i+=3)
		{
			dummy = indexes[i + 1];
			indexes[i + 1] = indexes[i + 2];
			indexes[i + 2] = dummy;

			// �e�N�X�`�����W�𔽓]
			for (int j = 0; j < 3; ++j) 
			{
				vertex[indexes[i + j]].TexCoord.x = 1.0f - vertex[indexes[i + j]].TexCoord.x;
				vertex[indexes[i + j]].Normal.x = -vertex[indexes[i + j]].Normal.x;
				vertex[indexes[i + j]].Normal.y = -vertex[indexes[i + j]].Normal.y;
				vertex[indexes[i + j]].Normal.z = -vertex[indexes[i + j]].Normal.z;
			}
		}
	}

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertex;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	Renderer::GetDevice()->CreateBuffer(&bd, &data, &g_VertexBuffer);

	// �C���f�b�N�X�o�b�t�@����
	// D3D11_BUFFER_DESC bd = {}; // ZeroMemory �Ɠ��������� �g���܂킷
	// bd.Usage = D3D11_USAGE_DEFAULT; // �g���܂킵�Ă�̂ŏ����Ȃ��Ă悢
	bd.ByteWidth = sizeof(unsigned short) * numIndex; // �o�b�t�@�T�C�Y�@�\���̃T�C�Y�~���_���� �P�ʓ�����O�p�`�Q��
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER; // �w��
	// bd.CPUAccessFlags = 0; // �g���܂킵�Ă�

	// D3D11_SUBRESOURCE_DATA data; // �g���܂킵
	data.pSysMem = indexes;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	Renderer::GetDevice()->CreateBuffer(&bd, &data, &g_IndexBuffer); // ���_�o�b�t�@�����

	// �e�N�X�`���ǂݍ���
	g_pTextureId = Texture::Load("asset\\texture\\kirby.png");

}

SpherePolygon::~SpherePolygon()
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

void SpherePolygon::Draw() const
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
	//GetDeviceContext()->Draw(m_DivisionV * (m_DivisionH + 1), 0); // Draw �̑�P�����͕\�����钸�_���̐�
	Renderer::GetDeviceContext()->DrawIndexed((m_DivisionH * 2 + m_DivisionH * (m_DivisionV - 2) * 2) * 3, 0, 0); // �C���f�b�N�X�ł̕`��
	//GetDeviceContext()->DrawIndexed(36, 0, 0); // �C���f�b�N�X�ł̕`��

}
