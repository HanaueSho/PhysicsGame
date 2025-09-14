/*
	CannonObject.h
	20250914 hanaue sho
	大砲のオブジェクト
*/
#ifndef CANNONOBJECT_H_
#define CANNONOBJECT_H_
#include "GameObject.h"

class AppleObject;

class CannonObject : public GameObject
{
private:
	std::vector<AppleObject*> m_pApples; // 発射用

public:
	void Init() override;
	void Update(float dt) override;

	void SetApples();
	void ShotApple();
};

#endif