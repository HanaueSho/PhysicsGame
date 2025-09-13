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
	pPlayer->Transform()->SetWorldPosition({ 5, 0, 0 });
	pPlayer->Transform()->SetEulerAngles({ 0, 0, 0 });
	
	for (int x = 0; x < 6; x++)
	{
		for (int z = 0; z < 6; z++)
		{
			pPlayer = AddGameObject<Player>(1);
			pPlayer->Init();
			pPlayer->Transform()->SetWorldPosition({ -6.0f + x * 2.0f, 0, -8.0f + z * 2.0f});
		}
	}

	// 床 ------
	//Field* pField = AddGameObject<Field>(1);
	//pField->Init();
	
	// エネミー -----
	Enemy* pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({0, 0, 0});
	pEnemy->Transform()->SetEulerAngles({30, 30, 30});
	//pEnemy->Transform()->SetWorldRotation(Quaternion::FromEulerAngles({0, 0, 0}));

	pPlayer->AddComponent<DebugRotateComponent>()->SetObject(pEnemy);

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

	/*										                 
	FollowCamera* pCamera = AddGameObject<FollowCamera>(0);

	Field* pField = AddGameObject<Field>(0);
	pField->Init();
	Player* pPlayer = AddGameObject<Player>(1);
	pPlayer->Init();
	pCamera->SetTarget(pPlayer);

	Enemy* pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->SetPosition({ 0, 0, 5 });
	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->SetPosition({ 4, 0, 5 });
	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->SetPosition({ -4, 0, 5 });

	Polygon2D* pPolygon = AddGameObject<Polygon2D>(2);
	pPolygon->Init({ 250, 0 ,0 }, { 0, 0, 90 }, {1, 1, 1});

	Score* pScore = AddGameObject<Score>(2);
	pScore->Init({ 250, 250 ,0 }, { 0, 0, 0 }, { 1, 1, 1 });

	// BGM再生
	m_pBGM = new Audio();
	m_pBGM->Load("assets\\audio\\bgm.wav");
	//m_pBGM->Play(true);
	*/

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
