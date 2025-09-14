/*
	AppleSpawnerObject.h
	20250913 hanaue sho
	リンゴのオブジェクト
*/
#ifndef APPLESPAWNEROBJECT_H_
#define APPLESPAWNEROBJECT_H_
#include "GameObject.h"

class AppleSpawnerObject : public GameObject
{
private:
	int m_AppleCount;
	float m_Timer = 4.0f;
	float m_Interval = 0.5f;
public:
	void Init() override;
	void Update(float dt) override;

};

#endif