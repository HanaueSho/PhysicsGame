/*
	QuadObject.h
	20250915 hanaue sho
	���ʂ̃I�u�W�F�N�g
*/
#ifndef QUADOBJECT_H_
#define QUADOBJECT_H_
#include "GameObject.h"

class QuadObject : public GameObject
{
private:
public:
	void Init() override;
	void Update(float dt) override;
};

#endif