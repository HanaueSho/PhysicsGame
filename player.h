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
#include "texture.h"  // Texture::Load Šù‘¶


class Audio;

class Player : public GameObject
{
private:
	GameObject* m_pCubes[5];
	class Audio* m_pSE;

	Vector3 m_Acceleration{};
	Vector3 m_Velocity{};
	const float MAX_VELOCITY = 0.1f;
	const float MAX_ACCE	 = 0.05f;
	bool m_IsMove = true;

	bool m_IsMoveTarget = false;
	Vector3 m_TargetPosition = { 0, 0, -8 };

public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

	void SetMove(bool b) { m_IsMove = b; }
	void MoveTarget(float dt);

};
