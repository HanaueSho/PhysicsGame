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

public:
	void Init() override;
	void Update(float dt) override;

	void SetApples();
	void ShotApple();
};

#endif