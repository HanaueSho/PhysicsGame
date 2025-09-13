/*
	CameraMove.h
	20250820  hanaue sho
*/
#include "CameraMove.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Vector3.h"

void CameraMove::Update(float dt)
{
	if (!m_pTransform) return;

	// �N���b�N���Ă���ԂɃ}�E�X�̈ړ��ʂ𔽉f -----
	Vector3 mouseDiff = Vector3(Mouse_GetPositionDiff().x, Mouse_GetPositionDiff().y, 0);
	Vector3 right	  = m_pTransform->Right();
	Vector3 up		  = m_pTransform->Up();
	Vector3 forward   = m_pTransform->Forward();

	// �E�N���b�N�ŉ�] -----
	if (Mouse_IsClick(MS_CLICK_RIGHT))
	{
		m_pTransform->Value().RotateAxis({ 0, 1, 0 }, mouseDiff.x * 0.005f);
		m_pTransform->Value().RotateAxis(right, mouseDiff.y * 0.005f);
	}
	// ���N���b�N�ňړ� -----
	if (Mouse_IsClick(MS_CLICK_MIDDLE))
	{
		Vector3 position = m_pTransform->Position();
		//printf("x:%f, y:%f, z:%f\n", position.x, position.y, position.z);
		position += right * mouseDiff.x * -0.01f;
		position += up * mouseDiff.y * 0.01f;
		m_pTransform->SetPosition(position);
	}
	// �z�C�[���őO��ړ� -----
	if (Mouse_IsWheelScroll())
	{
		Vector3 scroll = forward * Mouse_GetWheelScroll() * 0.5f; // ��������
		Vector3 position = m_pTransform->Position();
		//printf("x:%f, y:%f, z:%f\n", position.x, position.y, position.z);
		position += scroll;
		m_pTransform->SetPosition(position);
	}
}
