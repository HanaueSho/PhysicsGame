/*
	CylinderObject.h
	20250915 hanaue sho
	���̂̃I�u�W�F�N�g
*/
#ifndef CYLINDEROBJECT_H_
#define CYLINDEROBJECT_H_
#include "GameObject.h"

class CylinderObject : public GameObject
{
private:
public:
	void Init() override;
	void Update(float dt) override;
};

#endif