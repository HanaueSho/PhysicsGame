/*
	CannonObject.h
	20250914 hanaue sho
	��C�̃I�u�W�F�N�g
*/
#ifndef CANNONOBJECT_H_
#define CANNONOBJECT_H_
#include "GameObject.h"

class AppleObject;

class CannonObject : public GameObject
{
private:
	std::vector<AppleObject*> m_pApples; // ���˗p

	const float MAX_ANGLE = 1.5f;
	const float MIN_ANGLE = 0.8f;

	// �ړ��֌W
	Vector3 m_Acceleration{};
	Vector3 m_Velocity{};
	const float MAX_VELOCITY = 0.1f;
	const float MAX_ACCE = 0.05f;
	bool m_IsMove = true;
	bool m_IsShot = false;
	
	// SE
	class Audio* m_pSE = nullptr;

public:
	void Init() override;
	void Update(float dt) override;

	void SetApples();
	void ShotApple();
	void SetIsShot(bool b) { m_IsShot = b; }

	bool CheckEmptyApples() { return m_pApples.size() == 0; }
};

#endif