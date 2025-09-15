/*
	BillboardComponent.h
	20250915  hanaue sho
	�r���{�[�h�R���|�[�l���g
*/
#ifndef BILLBOARDCOMPONENT_H_
#define BILLBOARDCOMPONENT_H_
#include "Component.h"
#include "GameObject.h"
#include "Manager.h"
#include "Scene.h"
#include "Camera.h"


class BillboardComponent : public Component
{
public:
	enum class BillboardType
	{
		Spherical,		// ��ɃJ����������
		CylindericalY,  // Y�A�b�v��ۂ����������������킹��
		ScreenAligned	// �J�����ɐ���
	};

private:
	BillboardType m_Type = BillboardType::Spherical;
	bool m_Enable = true;

public:
	void SetType(BillboardType t) { m_Type = t; }
	void SetEnable(bool b) { m_Enable = b; }

	void Update(float dt) override
	{
		if (!m_Enable) return;
		auto* tf = Owner()->Transform();
		auto* cam = Manager::GetScene()->GetGameObject<Camera>();
		if (!tf || !cam) return;


		const Vector3 objPos = tf->WorldPosition();
		const auto* camTf = cam->Transform();
		const Vector3 camPos = camTf->WorldPosition();
		const Vector3 camFwd = camTf->Forward();
		const Vector3 worldUp = { 0, 1, 0 };

		Matrix4x4 rot;

		if (m_Type == BillboardType::Spherical) // +Z
		{
			Vector3 f = (camPos - objPos).normalized();
			if (f.lengthSq() < 1e-6f) return;
			Vector3 r = Vector3::Cross(worldUp, f).normalized();
			Vector3 u = Vector3::Cross(f, r);
			rot = Matrix4x4::CreateAxes(r, u, f);
		}
		else if (m_Type == BillboardType::CylindericalY) // Y �Œ�
		{
			Vector3 toCam = camPos - objPos;
			toCam.y = 0;					  // y�������O�ɂ���
			Vector3 f = toCam.normalized();
			if (f.lengthSq() < 1e-6f) return;
			Vector3 u = worldUp;
			Vector3 r = Vector3::Cross(u, f).normalized();
			rot = Matrix4x4::CreateAxes(r, u, f);
		}
		else if (m_Type == BillboardType::ScreenAligned) // ����
		{
			Vector3 f = camFwd;
			Vector3 r = Vector3::Cross(worldUp, f).normalized();
			Vector3 u = Vector3::Cross(f, r);
			rot = Matrix4x4::CreateAxes(r, u, f);
		}
		tf->SetWorldRotation(Quaternion::FromMatrix(rot));
	}
};



#endif