/*
	AppleObject.h
	20250913 hanaue sho
	リンゴのオブジェクト
*/
#ifndef APPLEOBJECT_H_
#define APPLEOBJECT_H_
#include "GameObject.h"

class AppleObject : public GameObject
{
private:
public:
	void Init() override;
	void Update(float dt) override;

};





#endif