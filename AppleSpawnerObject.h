/*
	AppleSpawnerObject.h
	20250913 hanaue sho
	�����S�̃I�u�W�F�N�g
*/
#ifndef APPLESPAWNEROBJECT_H_
#define APPLESPAWNEROBJECT_H_
#include "GameObject.h"

class AppleSpawnerObject : public GameObject
{
private:
	// 
	int m_AppleCount;
	float m_Timer = 0.0f;
	float m_Interval = 1.0f;
	float m_TimerSum = 0.0f;

	// �ړ�����
	float m_Sign = 1;

public:
	void Init() override;
	void Update(float dt) override;

};

#endif