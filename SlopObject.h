/*
	SlopObject.h
	20250913 hanaue sho
	リンゴのオブジェクト
*/
#ifndef SLOPOBJECT_H_
#define SLOPOBJECT_H_
#include "GameObject.h"

class SlopObject : public GameObject
{
private:
	GameObject* m_pObstancle[12];
	int m_Sign[4]{};

public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

};

#endif