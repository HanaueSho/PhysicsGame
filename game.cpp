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
#include "SlopObject.h"
#include "AppleSpawnerObject.h"
#include "TowerObject.h"
#include "CannonObject.h"

#include "DebugRotateComponent.h"
#include "Random.h"


void Game::Init()
{
	Scene::Init();

	// カメラ -----
	Camera* pCamera = AddGameObject<Camera>(0);
	pCamera->Init();
	pCamera->GetComponent<CameraComponent>()->SetMode(CameraComponent::Mode::Perspective);
	pCamera->GetComponent<CameraComponent>()->
		SetPerspective(DirectX::XMConvertToRadians(40.0f),(float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f);
	pCamera->GetComponent<TransformComponent>()->SetPosition({ 0, 10, -15});
	pCamera->GetComponent<TransformComponent>()->SetEulerAngles({ 0.1f, 0, 0 });
	
	// ライトの設定 -----
	LIGHT light = {};
	light.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.Direction = XMFLOAT4(0.2f, -1.0f, 0.8f, 0.0f);
	Renderer::SetLight(light);

	// ゲームモードセット -----
	SetGameMode(START);

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

	// スイッチステート
	switch (m_Mode)
	{
	case MODE::NONE:
		break;
	case MODE::START:
		break;
	case MODE::COLLECTION:
		break;
	case MODE::TO_SHOT:
		break;
	case MODE::SHOT:
		break;
	case MODE::GAMEOVER:
		break;
	}

	switch (m_NextMode)
	{
	case MODE::NONE:
		break;

	case MODE::START:
		m_Mode = START;
		// プレイヤー -----
		m_pPlayer = AddGameObject<Player>(1);
		m_pPlayer->Init();

		// スロープ -----
		m_pSlop = AddGameObject<SlopObject>(1);
		m_pSlop->Init();
		
		// アップルスポナー -----
		m_pSpawner = AddGameObject<AppleSpawnerObject>(1);
		m_pSpawner->Init();
		m_pSpawner = AddGameObject<AppleSpawnerObject>(1);
		m_pSpawner->Init();
		m_pSpawner->Transform()->SetPosition(m_pSpawner->Transform()->Position() + Vector3{ 1, 0, 0 });
		break;

	case MODE::COLLECTION:
		m_Mode = COLLECTION;
		break;

	case MODE::TO_SHOT:
		m_Mode = TO_SHOT;
		// 大砲生成 -----
		m_pCannon = AddGameObject<CannonObject>(1);
		m_pCannon->Init();

		// タワー生成 -----
		m_pTower = AddGameObject<TowerObject>(1);
		m_pTower->Init();
		dynamic_cast<TowerObject*>(m_pTower)->CreateTable({ 0, 3, 10 });
		dynamic_cast<TowerObject*>(m_pTower)->CreateTower(0, { 0, 0, 0 });
		break;

	case MODE::SHOT:
		m_Mode = SHOT;
		break;

	case MODE::GAMEOVER:
		m_Mode = GAMEOVER;
		break;
	}
	m_NextMode = MODE::NONE;

	if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		Manager::SetScene<Result>();
	}
}



/*
ゲームメモ

連続キャッチ
金リンゴ

カゴのふちに掬すくいあげ

リンゴがぷにっと膨らむ＋エフェクト



*/