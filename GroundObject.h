/*
	GroundObject.h
	20250914 hanaue sho
	�n�ʂ̃I�u�W�F�N�g
*/
#ifndef GROUNDOBJECT_H_
#define GROUNDOBJECT_H_
#include "GameObject.h"

class GroundObject : public GameObject
{
private:
public:
	void Init() override;
	void Update(float dt) override;

};

#endif