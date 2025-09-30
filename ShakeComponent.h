/*
	ShakeComponent.h
	20250926  hanaue sho
*/
#ifndef SHAKECOMPONENT_H_
#define SHAKECOMPONENT_H_
#include "Component.h"
#include "Vector3.h"
#include "GameObject.h"


class CameraShake : public Component
{
private:
	Vector3 m_BasePos = {};
	float m_ShakeTime = 0.0f;
	Vector3 m_ShakeVector = {};
	bool m_IsShaking = false;

public:
	void Update(float dt) override
	{
		if (!m_IsShaking) return;

		Vector3 pos = m_BasePos;
		pos += m_ShakeVector * cosf(m_ShakeTime);
		Owner()->Transform()->SetPosition(pos);

		m_ShakeTime -= dt;
		if (m_ShakeTime < 0.0f)
			m_IsShaking = false;
		m_ShakeVector *= 0.5f;
	}
	void Shake(const Vector3& vect)
	{
		m_ShakeVector = vect;
		m_ShakeTime = 1.0f;
		if (!m_IsShaking)  // 揺れてないときだけ位置を更新
		{
			m_BasePos = Owner()->Transform()->Position();
			printf("位置更新\n");
		}
		m_IsShaking = true;
	}
};

#endif