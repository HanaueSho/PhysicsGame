/*
	TreeObject.cpp
	20250913 hanaue sho
	リンゴのオブジェクト
*/
#include "TreeObject.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "renderer.h"
#include "MathCommon.h"
#include "texture.h"  // Texture::Load 既存

#include "Manager.h"
#include "Scene.h"
#include "CylinderObject.h"
#include "SphereObject.h"

void TreeObject::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0, 0, 0 });
	tf->SetScale({ 1, 1, 1 });
	tf->SetEulerAngles({ 1, 1, 1 });

	Vector3 worldPos = { -18, 0, 10 };

	GameObject* wood = Manager::GetScene()->AddGameObject<CylinderObject>(1);
	wood->Init();
	wood->Transform()->SetScale({ 10, 100, 10 });
	wood->Transform()->SetPosition({ worldPos.x + 0, worldPos.y + 40, worldPos.z + 0 });

	GameObject* leaf = Manager::GetScene()->AddGameObject<SphereObject>(1);
	leaf->Init();
	leaf->Transform()->SetScale({ 40, 18, 40 });
	leaf->Transform()->SetPosition({ worldPos.x + 0, worldPos.y + 80, worldPos.z + 0 });
}

