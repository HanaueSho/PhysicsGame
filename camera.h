/*
	Camera.h
	20250818 hanaue sho
*/
#ifndef CAMERA_H_
#define CAMERA_H_
#include "GameObject.h"
#include "TransformComponent.h"
#include "CameraComponent.h"

// ----- MyCameraClass -----
// --------------------------------------------------
// ƒJƒƒ‰Šî’êƒNƒ‰ƒX
// --------------------------------------------------
class Camera : public GameObject
{
public:
	void Init() override;
};

#endif