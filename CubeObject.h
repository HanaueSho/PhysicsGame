/*
	CubeObject.h
	20250913 hanaue sho
	キューブのオブジェクト
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