/*
	CubeObject.h
	20250913 hanaue sho
	�L���[�u�̃I�u�W�F�N�g
*/
#ifndef CUBEOBJECT_H_
#define CUBEOBJECT_H_
#include "GameObject.h"

class CubeObject : public GameObject
{
private:
public:
	void Init() override ;
	void Update(float dt) override;	
};

#endif