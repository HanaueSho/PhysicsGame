/*
	Game.cpp
	20250625  hanaue sho
*/
#include "game.h"
#include "manager.h"
#include "renderer.h"
#include "keyboard.h"
#include "result.h"

#include "polygon.h"
#include "field.h"
#include "camera.h"
#include "player.h"
#include "enemy.h"
#include "score.h"
#include "audio.h"
#include "renderer.h"
#include "AppleObject.h"

#include "DebugRotateComponent.h"


void Game::Init()
{
	Scene::Init();

	// カメラ -----
	Camera* pCamera = AddGameObject<Camera>(0);
	pCamera->Init();
	pCamera->GetComponent<CameraComponent>()->SetMode(CameraComponent::Mode::Perspective);
	pCamera->GetComponent<CameraComponent>()->
		SetPerspective(DirectX::XMConvertToRadians(60.0f),(float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f);
	pCamera->GetComponent<TransformComponent>()->SetPosition({ 0, 3, -15 });
	pCamera->GetComponent<TransformComponent>()->SetEulerAngles({ 0.5, 0, 0 });
	
	// プレイヤー -----
	Player* pPlayer = AddGameObject<Player>(1);
	pPlayer->Init();
	
	// エネミー -----
	Enemy* pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetScale({ 1.0f, 1.0f, 1.0f });
	pEnemy->Transform()->SetPosition({0, 0, 5});
	pEnemy->Transform()->SetEulerAngles({1.7f, 1.7f, 1.7f });
	//pEnemy->Transform()->SetWorldRotation(Quaternion::FromEulerAngles({0, 0, 0}));

	//pEnemy->Transform()->SetParent(pPlayer->Transform()); 

	//pEnemy = AddGameObject<Enemy>(1);  
	//pEnemy->Init();
	//pEnemy->Transform()->SetPosition({-2.0f, 0, 0});
	//pEnemy->Transform()->SetEulerAngles({ 0, 0, 0 });

	// 床
	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({ 0, -10, 0 });
	pEnemy->Transform()->SetScale({ 10, 5, 10 });
	pEnemy->Transform()->SetEulerAngles({ 0, 0, 0 });
	pEnemy->GetComponent<Collider>()->SetBox({ 1, 1, 1 });
	pEnemy->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static); // 壁

	// 壁４方
	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({ 10.5f, -4, 0 });
	pEnemy->Transform()->SetScale({ 1, 1, 10 });
	pEnemy->GetComponent<Collider>()->SetBox({ 1, 1, 1 });
	pEnemy->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static); // 壁
	
	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({ -10.5f, -4, 0 });
	pEnemy->Transform()->SetScale({ 1, 1, 10 });
	pEnemy->GetComponent<Collider>()->SetBox({ 1, 1, 1 });
	pEnemy->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static); // 壁

	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({ 0, -4, 10.5f });
	pEnemy->Transform()->SetScale({ 10, 1, 1 });
	pEnemy->GetComponent<Collider>()->SetBox({ 1, 1, 1 });
	pEnemy->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static); // 壁

	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({ 0, -4, -10.5f });
	pEnemy->Transform()->SetScale({ 10, 1, 1 });
	pEnemy->GetComponent<Collider>()->SetBox({ 1, 1, 1 });
	pEnemy->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static); // 壁

	for (int x = 0; x < 1; x++)
	{
		for (int z = 0; z < 2; z++)
		{
			AppleObject* pApple = AddGameObject<AppleObject>(1);
			pApple->Init();
			pApple->Transform()->SetWorldPosition({ -0.0f + x * 0.5f, 0, -0.0f + z * 0.5f });
		}
	}



	// ライトの設定 -----
	LIGHT light = {};
	light.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.Direction = XMFLOAT4(0.2f, -1.0f, 0.8f, 0.0f);
	Renderer::SetLight(light);

}

void Game::Uninit()
{
	// BGM停止
	//m_pBGM->Uninit();
	//delete m_pBGM;

	Scene::Uninit();
}

void Game::Update(float dt)
{
	Scene::Update(dt);	

	if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		Manager::SetScene<Result>();
	}
}
