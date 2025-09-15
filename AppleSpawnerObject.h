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
	// 
	int m_AppleCount;
	float m_Timer = 0.0f;
	float m_Interval = 1.0f;
	float m_TimerSum = 0.0f;

	// 移動処理
	float m_Sign = 1;

public:
	void Init() override;
	void Update(float dt) override;

};

#endif