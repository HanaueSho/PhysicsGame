/*
	SkydomeObject.h
	20250914 hanaue sho
	�X�J�C�h�[���̃I�u�W�F�N�g
*/
#ifndef SKYDOMEOBJECT_H_
#define SKYDOMEOBJECT_H_
#include "GameObject.h"

class SkydomeObject : public GameObject
{
private:
public:
	void Init() override;
	void Update(float dt) override;

};

#endif