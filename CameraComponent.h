/*
	CameraComponent.h
	20250813  hanaue sho
	�J�����Ɏ���������
	���͎����Matrix4x4�ł͂Ȃ�XMMatrix���g���Ă����
*/
#ifndef CAMERACOMPONENT_H_
#define CAMERACOMPONENT_H_
#include "Component.h"
#include "TransformComponent.h"
#include "Renderer.h"
#include <DirectXMath.h>
#include "GameObject.h"


class CameraComponent : public Component
{
public:
	enum class Mode { Perspective, Ortho2D };

protected:
	// �ˑ�
	TransformComponent* m_pTransform = nullptr;

	// ����
	Mode m_Mode		 = Mode::Perspective;
	bool m_IsMain	 = true; // �Ƃ肠�������C���J�����ɐݒ�
	bool m_ProjDirty = true; // �ˉe�p�����[�^�ύX


	// �������e�iPerspective�j
	float m_FovY = DirectX::XMConvertToRadians(60.0f);
	float m_Aspect = 16.0f / 9.0f;
	float m_NearZ = 0.1f;
	float m_FarZ = 1000.0f;

	// ���s���e�iOrthographic, ���㌴�_�AY�������j
	float m_OrthoLeft	= 0.0f;
	float m_OrthoRight  = static_cast<float>(SCREEN_WIDTH);
	float m_OrthoTop	= 0.0f;
	float m_OrthoBottom = static_cast<float>(SCREEN_HEIGHT);
	float m_OrthoNearZ = 0.0f;
	float m_OrthoFarZ = 1.0f;
	bool m_UseTransformIn2D = false; // 2D�ł�Transform�Ńp���A�Y�[���������ꍇ��true

public:
	// ------ �ݒ� -----
	void SetMode(Mode m) noexcept { if (m_Mode != m) { m_Mode = m; m_ProjDirty = true; } }
	void SetAsMain(bool v) noexcept { m_IsMain = v; }

	// ----- �������e -----
	void SetPerspective(float fovYRadian, float aspect, float nearZ, float farZ) noexcept
	{
		m_FovY = fovYRadian; m_Aspect = aspect;	m_NearZ = nearZ; m_FarZ = farZ;
		m_ProjDirty = true;
	}
	void SetFovY(float fovY)				 noexcept { m_FovY = fovY; m_ProjDirty = true; }
	void SetAspect(float aspect)			 noexcept { m_Aspect = aspect; m_ProjDirty = true; }
	void SetNearFar(float nearZ, float farZ) noexcept { m_NearZ = nearZ; m_FarZ = farZ; m_ProjDirty = true; }

	// ----- ���s���e -----
	void SetOrthoOffCenter(float left, float right, float bottom, float top, float nearZ = 0.0f, float farZ = 1.0f) noexcept
	{
		m_OrthoLeft = left; m_OrthoRight = right; m_OrthoTop = top; m_OrthoBottom = bottom;
		m_OrthoNearZ = nearZ; m_OrthoFarZ = farZ; m_ProjDirty = true;
	}
	void UseTransformIn2D(bool v) noexcept { m_UseTransformIn2D = v; }

	// ----- ���C�t�T�C�N�� -----
	void OnAdded() override
	{
		m_pTransform = Owner()->GetComponent<TransformComponent>();
		m_ProjDirty = true;
	}

	void Update(float dt) override
	{
		if (!m_IsMain) return;
		
		// ----- View -----
		if (m_Mode == Mode::Perspective) UpdateView3D();
		else UpdateView2D();

		// ----- Projection -----
		if (m_ProjDirty) // �ύX���������Ƃ��������������
		{
			if (m_Mode == Mode::Perspective)
			{
				DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(m_FovY, m_Aspect, m_NearZ, m_FarZ);
				Renderer::SetProjectionMatrix(projection);
			}
			else
			{
				DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterLH(m_OrthoLeft, m_OrthoRight, m_OrthoBottom, m_OrthoTop, m_OrthoNearZ, m_OrthoFarZ);
				Renderer::SetProjectionMatrix(projection);
			}
			m_ProjDirty = false;
		}
	}

private:
	void UpdateView3D()
	{
		if (!m_pTransform) return; // �ύX�������̂ŏ������΂��i!m_pTransform->IsWorldDirty()�͈�U�Ȃ��ɂ���j

		// View(LH) :�ʒu��Forward/Up���琶��
		const Vector3 pos = m_pTransform->Position();
		Vector3 forward = m_pTransform->Forward().normalized();
		Vector3 up = m_pTransform->Up().normalized();
		if (fabsf(Vector3::Dot(forward, up)) > 0.999f) {
			// forward �Ƃقڕ��s�Ȃ�ʂ̎���I��
			if (fabsf(forward.y) > 0.999f) 
			{
				// forward ���ق�Y���Ȃ� X�����g��
				up = Vector3(1, 0, 0);
			}
			else 
			{
				// ����ȊO�� Y�����g��
				up = Vector3(0, 1, 0);
			}
		}

		using namespace DirectX;
		XMMATRIX view = XMMatrixLookToLH(
			XMVectorSet(pos.x, pos.y, pos.z, 1.0f),
			XMVectorSet(forward.x, forward.y, forward.z, 0.0f),
			XMVectorSet(up.x, up.y, up.z, 0.0f)
		);

		// Renderer ��
		Renderer::SetViewMatrix(view);

		// Dirty ������iWorldMatrix() ������ worldDirty = false �ɂ���݌v�j
		(void)m_pTransform->WorldMatrix();
	}

	void UpdateView2D()
	{
		if (!m_UseTransformIn2D || !m_pTransform)
		{
			// 2D�Œ�J�����iView = identity�j
			Renderer::SetViewMatrix(DirectX::XMMatrixIdentity());
			return;
		}

		// 2D�Ńp���������ꍇ�F�J�����ʒu C ���g���� View = Translate(-C)
		const Vector3 pos = m_pTransform->Position();
		const DirectX::XMMATRIX view = DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z);
		Renderer::SetViewMatrix(view);
		(void)m_pTransform->WorldMatrix();
	}
};

#endif