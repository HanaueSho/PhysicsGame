/*
	SlopObject.h
	20250913 hanaue sho
	�����S�̃I�u�W�F�N�g
*/
#ifndef SLOPOBJECT_H_
#define SLOPOBJECT_H_
#include "GameObject.h"

class SlopObject : public GameObject
{
private:
public:
	void Init() override;
	void Update(float dt) override;

};

#endif