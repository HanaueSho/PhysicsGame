/*
	SphereObject.h
	20250915 hanaue sho
	���̂̃I�u�W�F�N�g
*/
#ifndef SPHEREOBJECT_H_
#define SPHEREOBJECT_H_
#include "GameObject.h"

class SphereObject : public GameObject
{
private:
public:
	void Init() override;
	void Update(float dt) override;
};

#endif