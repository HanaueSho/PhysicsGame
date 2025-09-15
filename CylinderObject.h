/*
	CylinderObject.h
	20250915 hanaue sho
	球体のオブジェクト
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