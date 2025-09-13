/*
    common.hlsl
20250423 hanaue sho
*/
// ���̃t�@�C���͑��̃V�F�[�_�[�t�@�C���փC���N���[�h�����
// �e��}�g���N�X��x�N�g�����󂯎��ϐ���p��

cbuffer WorldBuffer : register(b0) // �萔�o�b�t�@�O�� �s����󂯎��萔�o�b�t�@
{
    matrix World; // matrix �^�� World �֎󂯎��
}

cbuffer ViewBuffer : register(b1) // �萔�o�b�t�@�P��
{
    matrix View;
}

cbuffer ProjectionBuffer : register(b2) // �萔�o�b�t�@�Q��
{
    matrix Projection;
}
struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    bool TextureEnable;
    float2 Dummy;
};
    
// ���_�V�F�[�_�\�֓��͂����f�[�^���\���̂̌`�ŕ\��
struct VS_IN
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

// �s�N�V�F���V�F�[�_�[�֓��͂����f�[�^���\����̌`�ŕ\��
struct PS_IN
{
    float4 Position : SV_Position;
    float4 WorldPosition : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
    MATERIAL Material : MATERIAL;
};

struct LIGHT
{
    bool Enable; // bool �^�Ƃ͌����������I�ɂ� float �^
    bool3 Dummy; // �z�u�A�h���X������̔{���ɂ��邽�߂̃p�f�B���O
    float4 Direction; // ����C����ł������� VisualStudio ������Ă���Ă���i�����H�j
    float4 Diffuse;
    float4 Ambient;
    
    float4 Position; // ���̈ʒu
    float4 PointLightParam; // ���̓͂�����
};

cbuffer LightBuffer : register(b4) // �R���X�^���g�o�b�t�@�S�ԂƂ���
{
    LIGHT Light; // LIGHT �\����
}

cbuffer CameraBuffer : register(b5) // �o�b�t�@�̂T�ԂƂ���
{
    float4 CameraPosition; // �J�����̍��W���󂯎��ϐ�
}

cbuffer ParameterBuffer : register(b6)
{
    float4 Parameter; // �V�F�[�_�[���Ŏg���ϐ���
}
