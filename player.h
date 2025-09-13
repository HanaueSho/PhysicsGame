/*
	player.h
	20250514 hanaue sho
*/
#pragma once
#include "gameObject.h"

#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "renderer.h"
#include "texture.h"  // Texture::Load ����


class Audio;

class Player : public GameObject
{
private:
	class Audio* m_pSE;

public:
	void Init() override;
	void Update(float dt) override;
};
