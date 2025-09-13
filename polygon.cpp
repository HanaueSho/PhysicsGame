/*
	polygon.cpp
	20250423 hanaue sho
*/
#include "polygon.h"

void Polygon2D::Init()
{
    // 1) Transform�i���� GameObject ctor �Œǉ��ς݁j���擾���ď����p�������Ă���
    auto* tf = GetComponent<TransformComponent>();
    tf->SetPosition({ 0,0,0 });
    tf->SetScale({ 1,1,1 });
    tf->SetEulerAngles({ 0,0,0 });

    // 2) MeshFilter ��ǉ����Ē��_�o�b�t�@�i4���_�̋�`�j�����
    auto* mf = AddComponent<MeshFilterComponent>();
    MeshFactory::CreateQuad(mf, { 200.0f, 200.0f, false });

    // 3) Material ��ǉ��i�V�F�[�_/�e�N�X�`��/�}�e���A���j
    auto* mat = AddComponent<MaterialComponent>();

    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    ID3D11InputLayout* il = nullptr;
    Renderer::CreateVertexShader(&vs, &il, "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&ps, "shader\\unlitTexturePS.cso");
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

}

