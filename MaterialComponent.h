/*
	MaterialComponent.h
	20250813  hanaue sho
	�h�ǂ��`�����h�Ƃ����������i�V�F�[�_�[�A�}�e���A���A�e�N�X�`���A�T���v���[�j
*/
#ifndef MATERIALCOMPONENT_H_
#define MATERIALCOMPONENT_H_
#include "Component.h"
#include "Renderer.h"
#include <d3d11.h>

class MaterialComponent : public Component
{
public:
	enum class BlendMode {Opaque, Alpha, Additive};
protected:
	// �V�F�[�_�[���C�A�E�g�i�f�t�H���g�F���L���遁�������j
	ID3D11VertexShader* m_VS = nullptr;
	ID3D11PixelShader*  m_PS = nullptr;
	ID3D11InputLayout*  m_IL = nullptr;
	bool m_OwnsVS = true;
	bool m_OwnsPS = true;
	bool m_OwnsIL = true;

	MATERIAL	m_Material{}; // Renderer.h ��MATERIAL

	// �e�N�X�`���i�ʏ�̓A�Z�b�g���L���񏊗L�j
	ID3D11ShaderResourceView* m_MainTexture = nullptr;
	ID3D11SamplerState*		  m_Sampler		= nullptr; 
	bool m_OwnsSRV = false;
	bool m_OwnsSamp = false;

	// �u�����h�ݒ�
	BlendMode m_Blend = BlendMode::Opaque;

public:
	// ----- �Z�b�g -----
	void SetVSPS(ID3D11VertexShader* vs, ID3D11PixelShader* ps, ID3D11InputLayout* il, bool takeVS = true, bool takePS = true, bool takeIL = true ) noexcept
	{	
		// �������L���̒u���������̓��[�N�h�~
		if (m_OwnsVS && m_VS && m_VS != vs) m_VS->Release();
		if (m_OwnsPS && m_PS && m_PS != ps) m_PS->Release();
		if (m_OwnsIL && m_IL && m_IL != il) m_IL->Release();

		m_VS = vs; m_PS = ps; m_IL = il; 
		m_OwnsVS = takeVS; m_OwnsPS = takePS; m_OwnsIL = takeIL;
	}
	void SetMaterial(const MATERIAL& m) noexcept { m_Material = m; }
	const MATERIAL& GetMaterial() const noexcept { return m_Material; }

	void SetMainTexture(ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler, bool takeSrv = false, bool takeSamp = false) noexcept
	{ 
		if (m_OwnsSRV && m_MainTexture && m_MainTexture != srv) m_MainTexture->Release();
		if (m_OwnsSamp && m_Sampler && m_Sampler != sampler)	m_Sampler->Release();

		m_MainTexture = srv; m_Sampler = sampler; 
		m_OwnsSRV = takeSrv; m_OwnsSamp = takeSamp;
	}

	// ----- �Ó��� -----
	bool IsReady() const noexcept {	return (m_VS && m_PS && m_IL); }

	// ----- �o�C���h -----
	void Bind() const
	{
		auto* ctx = Renderer::GetDeviceContext();
		if (!ctx || !IsReady()) return;

		ctx->IASetInputLayout(m_IL);
		ctx->VSSetShader(m_VS, nullptr, 0);
		ctx->PSSetShader(m_PS, nullptr, 0);
		
		// �u�����h�A�[�x�ݒ�
		switch (m_Blend)
		{
		case BlendMode::Opaque:
			Renderer::SetATCEnable(false);
			Renderer::SetDepthEnable(true); // �[�x��������ON�O��
			break;
		case BlendMode::Alpha:
		case BlendMode::Additive:
			Renderer::SetATCEnable(true);
			Renderer::SetDepthEnable(false); // �[�x��������OFF�O��
			break;
		}

		Renderer::SetMaterial(m_Material);

		if (m_MainTexture) ctx->PSSetShaderResources(0, 1, &m_MainTexture);
		if (m_Sampler)	   ctx->PSSetSamplers(0, 1, &m_Sampler);
	}

	// �e�N�X�`���𖾎��I�ɊO��
	void UnbindTextures() const noexcept
	{
		auto* ctx = Renderer::GetDeviceContext();
		if (!ctx) return;
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		ctx->PSSetShaderResources(0, 1, nullSRV);
	}

	// �u�����h�ݒ�
	void SetBlendMode(BlendMode m) noexcept { m_Blend = m; }
	bool IsTransparent() const noexcept { return m_Blend != BlendMode::Opaque; }

	// ----- �j�� -----
	void Uninit() override
	{
		if (m_OwnsVS && m_VS) { m_VS->Release(); m_VS = nullptr; }
		if (m_OwnsPS && m_PS) { m_PS->Release(); m_PS = nullptr; }
		if (m_OwnsIL && m_IL) { m_IL->Release(); m_IL = nullptr; }
		if (m_OwnsSRV && m_MainTexture) { m_MainTexture->Release(); m_MainTexture = nullptr; }
		if (m_OwnsSamp && m_Sampler)	{ m_Sampler->Release();     m_Sampler = nullptr; }
	}
};

#endif