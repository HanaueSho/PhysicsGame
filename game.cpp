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
#include "SkydomeObject.h"
#include "GroundObject.h"
#include "QuadObject.h"
#include "SphereObject.h"
#include "CylinderObject.h"
#include "TreeObject.h"
#include "FadeObject.h"
#include "title.h"

#include "DebugRotateComponent.h"
#include "Random.h"


void Game::Init()
{
	Scene::Init();

	// カメラ -----
	m_pCamera = AddGameObject<Camera>(0);
	m_pCamera->Init();
	m_pCamera->GetComponent<CameraComponent>()->SetMode(CameraComponent::Mode::Perspective);
	m_pCamera->GetComponent<CameraComponent>()->
		SetPerspective(DirectX::XMConvertToRadians(40.0f),(float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f);
	m_pCamera->GetComponent<TransformComponent>()->SetPosition({ 0, 8, -17});
	m_pCamera->GetComponent<TransformComponent>()->SetEulerAngles({ 0.1f, 0, 0 });
	
	// スカイドーム、地面 -----
	SkydomeObject* pSkydome = AddGameObject<SkydomeObject>(1);
	pSkydome->Init();
	GroundObject* pGround = AddGameObject<GroundObject>(1);
	pGround->Init();

	// ライトの設定 -----
	LIGHT light = {};
	light.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.Direction = XMFLOAT4(0.2f, -1.0f, 0.8f, 0.0f);
	Renderer::SetLight(light);

	// ゲームモードセット -----
	SetGameMode(START);

	// フェード設定 -----
	m_pFadeQuad = AddGameObject<FadeObject>(1);
	m_pFadeQuad->Init();
	m_pFadeQuad->Transform()->SetPosition({0, 5, -10});
	dynamic_cast<FadeObject*>(m_pFadeQuad)->SetFade(FadeObject::Fade::In);
	dynamic_cast<FadeObject*>(m_pFadeQuad)->SetFadeTime(2.0f);

	GameObject* pTree = AddGameObject<TreeObject>(1);
	pTree->Init();


	// BGM再生
	m_pBGM = new Audio();
	m_pBGM->Load("assets\\audio\\BGMgame.wav");
	m_pBGM->Play(true);

	m_pSEgameclear= new Audio();
	m_pSEgameover = new Audio();
	m_pSEtimeup   = new Audio();
	m_pSEgameclear->Load("assets\\audio\\gameclear.wav");
	m_pSEgameover ->Load("assets\\audio\\gameover.wav");
	m_pSEtimeup   ->Load("assets\\audio\\timeup.wav");
	m_pSEenter = new Audio();
	m_pSEenter->Load("assets\\audio\\enter.wav");
}

void Game::Uninit()
{
	// BGM停止
	m_pBGM->Uninit();
	delete m_pBGM;
	m_pSEgameclear->Uninit();
	delete m_pSEgameclear;
	m_pSEgameover ->Uninit();
	delete m_pSEgameover ;
	m_pSEtimeup   ->Uninit();
	delete m_pSEtimeup   ;
	m_pSEenter->Uninit();
	delete m_pSEenter;

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
		if (Keyboard_IsKeyDownTrigger(KK_ENTER)) // エンターキーでスタート
		{
			SetGameMode(COLLECTION);
			m_pSEenter->Play(false);
		}
		break;

	case MODE::COLLECTION:
		/*if (Keyboard_IsKeyDownTrigger(KK_ENTER))
		{
			SetGameMode(TO_SHOT_1);
		}*/
		m_Timer += dt;
		if (m_Timer > m_TimeColl + m_Interval)
		{
			SetGameMode(TO_SHOT_1);
		}
		break;

	case MODE::TO_SHOT_1:
		m_Timer += dt;

		// カメラ制御 -----
		{ // 上を向ける
			float targetAngle = - PI * 0.5f;
			Vector3 angle = m_pCamera->Transform()->EulerAngles();
			float diff = targetAngle - angle.x;
			diff *= dt;
			angle.x += diff;
			m_pCamera->Transform()->SetEulerAngles(angle);
		}
		{
			float targetFov = 60.0f;
			float fov =	m_pCamera->GetComponent<CameraComponent>()->FovY() * 360.0f / (2 * PI);
			float diff = targetFov - fov;
			diff *= dt;
			fov += diff;
			m_pCamera->GetComponent<CameraComponent>()->
				SetPerspective(DirectX::XMConvertToRadians(fov), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f);
			printf("fov = %f\n", fov);
		}
		if (m_Timer > m_TimeToShot)
		{
			SetGameMode(TO_SHOT_2);

			m_pCamera->GetComponent<CameraComponent>()->
				SetPerspective(DirectX::XMConvertToRadians(60.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f);
		}
		break;

	case MODE::TO_SHOT_2:
		m_Timer += dt;

		//　プレイヤー制御 -----
		dynamic_cast<Player*>(m_pPlayer)->MoveTarget(dt);

		// カメラ制御 -----
		// 位置移動、角度
		{
			float targetAngle = PI * 0.1f;
			Vector3 angle = m_pCamera->Transform()->EulerAngles();
			float diff = targetAngle - angle.x;
			diff *= dt * 2.0f;
			angle.x += diff;
			m_pCamera->Transform()->SetEulerAngles(angle);
		}
		{
			Vector3 targetPos = {0, 6.2f, -17};
			Vector3 pos = m_pCamera->Transform()->Position();
			Vector3 diff = targetPos - pos;
			diff *= dt;
			pos += diff;
			m_pCamera->Transform()->SetPosition(pos);
		}

		if (m_Timer > m_TimeToShot + 1)
		{
			SetGameMode(SHOT);
			Vector3 angle = m_pCamera->Transform()->EulerAngles();
			angle.x = PI * 0.1f;
			m_pCamera->Transform()->SetEulerAngles(angle);
			m_pCamera->Transform()->SetPosition({ 0, 6.2f, -17 });
		}
		break;

	case MODE::SHOT:
		dynamic_cast<Player*>(m_pPlayer)->MoveTarget(dt);

		if (dynamic_cast<CannonObject*>(m_pCannon)->CheckEmptyApples())
		{
			SetGameMode(GAMEOVER);
		}

		if (dynamic_cast<TowerObject*>(m_pTower_1)->CheckBlocks() &&
			dynamic_cast<TowerObject*>(m_pTower_2)->CheckBlocks() &&
			dynamic_cast<TowerObject*>(m_pTower_3)->CheckBlocks() 			
			)
		{
			SetGameMode(GAMECLEAR);
		}
		break;

	case MODE::GAMEOVER:
	case MODE::GAMECLEAR:
		m_Timer += dt;

		if (m_Timer > 3)
		{
			SetGameMode(TO_TITLE);
		}
		break;
	case MODE::TO_TITLE:
		m_Timer += dt;

		if (m_Timer > 3)
		{
			Manager::SetScene<Title>();
		}
		break;
	}
	
	// ====================================================================================================

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

		// スタートテキスト -----
		{
			m_pText[0] = Manager::GetScene()->AddGameObject<QuadObject>(1);
			m_pText[0]->Init();
			ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\textStart.png");
			m_pText[0]->GetComponent<MaterialComponent>()->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
			m_pText[0]->Transform()->SetPosition({ 0, 7, -5 });
			m_pText[0]->Transform()->SetScale({ 10, 10, 1 });
		}
		break;

	case MODE::COLLECTION:
		m_Mode = COLLECTION;

		// 破棄 -----
		m_pText[0]->RequestDestroy();

		// アップルスポナー -----
		m_pSpawner[0] = AddGameObject<AppleSpawnerObject>(1);
		m_pSpawner[0]->Init();
		m_pSpawner[1] = AddGameObject<AppleSpawnerObject>(1);
		m_pSpawner[1]->Init();
		m_pSpawner[1]->Transform()->SetPosition(m_pSpawner[1]->Transform()->Position() + Vector3{1, 0, 0});

		// カゴテキスト -----
		{
			m_pText[1] = Manager::GetScene()->AddGameObject<QuadObject>(1);
			m_pText[1]->Init();
			ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\textBasket.png");
			m_pText[1]->GetComponent<MaterialComponent>()->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
			m_pText[1]->Transform()->SetPosition({ 8, 10, 0 });
			m_pText[1]->Transform()->SetScale({ 5, 5, 1 });
		}
		break;

	case MODE::TO_SHOT_1:
		m_Mode = TO_SHOT_1;
		m_Timer = 0;

		// 破棄 -----
		m_pSpawner[0]->RequestDestroy();
		m_pSpawner[1]->RequestDestroy();
		m_pText[1]->RequestDestroy();

		// プレイヤー制御 ----
		dynamic_cast<Player*>(m_pPlayer)->SetMove(false);

		m_pSEtimeup->Play(false);

		break;
	
	case MODE::TO_SHOT_2:
		m_Mode = TO_SHOT_2;
		m_Timer = 0;

		// 破棄 -----
		m_pSlop->RequestDestroy();

		// 大砲生成 -----
		m_pCannon = AddGameObject<CannonObject>(1);
		m_pCannon->Init();

		// タワー生成 -----
		m_pTower_1 = AddGameObject<TowerObject>(1);
		m_pTower_1->Init();
		dynamic_cast<TowerObject*>(m_pTower_1)->CreateTable({ 0, 3, 0 });
		dynamic_cast<TowerObject*>(m_pTower_1)->CreateTower(0, { 0, 0.1f, 0 });
		dynamic_cast<TowerObject*>(m_pTower_1)->CreateTower(3, { -2, -0.1, 0 });
		dynamic_cast<TowerObject*>(m_pTower_1)->CreateTower(6, { 2, -0.1, 0 });
		m_pTower_2 = AddGameObject<TowerObject>(1);
		m_pTower_2->Init();
		dynamic_cast<TowerObject*>(m_pTower_2)->CreateTable({ -1, 7, 0 });
		dynamic_cast<TowerObject*>(m_pTower_2)->CreateTower(0, {     0,  0.1f, 0 });
		dynamic_cast<TowerObject*>(m_pTower_2)->CreateTower(3, { -1.1f, -0.1f, 0 });
		dynamic_cast<TowerObject*>(m_pTower_2)->CreateTower(6, {  1.1f, -0.1f, 0 });
		m_pTower_3 = AddGameObject<TowerObject>(1);
		m_pTower_3->Init();
		dynamic_cast<TowerObject*>(m_pTower_3)->CreateTable({  1, 7, 0 });
		dynamic_cast<TowerObject*>(m_pTower_3)->CreateTower(0, {     0,  0.1f, 0 });
		dynamic_cast<TowerObject*>(m_pTower_3)->CreateTower(3, { -1.1f, -0.1f, 0 });
		dynamic_cast<TowerObject*>(m_pTower_3)->CreateTower(6, {  1.1f, -0.1f, 0 });
		
		// カゴテキスト -----
		{
			m_pText[2] = Manager::GetScene()->AddGameObject<QuadObject>(1);
			m_pText[2]->Init();
			ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\textCannon.png");
			m_pText[2]->GetComponent<MaterialComponent>()->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
			m_pText[2]->Transform()->SetPosition({ 8, 7, -5 });
			m_pText[2]->Transform()->SetScale({ 5, 5, 1 });
		}

		break;

	case MODE::SHOT:
		m_Mode = SHOT;
		m_Timer = 0;

		// リンゴセット -----
		dynamic_cast<CannonObject*>(m_pCannon)->SetApples();
		dynamic_cast<CannonObject*>(m_pCannon)->SetIsShot(true);

		// 床に当たり判定を作る -----
		Manager::GetScene()->GetGameObject<GroundObject>()->GetComponent<Collider>()->SetTrigger(false);
		break;

	case MODE::GAMEOVER:
		m_Mode = GAMEOVER;
		m_Timer = 0;

		// テキスト -----
		{
			m_pText[3] = Manager::GetScene()->AddGameObject<QuadObject>(1);
			m_pText[3]->Init();
			ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\textGameOver.png");
			m_pText[3]->GetComponent<MaterialComponent>()->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
			m_pText[3]->Transform()->SetPosition({ 0, 4, -9 });
			m_pText[3]->Transform()->SetScale({ 10, 10, 1 });
		}

		m_pSEgameover->Play(false);
		break;
	case MODE::GAMECLEAR:
		m_Mode = GAMECLEAR;
		m_Timer = 0;

		// テキスト -----
		{
			m_pText[4] = Manager::GetScene()->AddGameObject<QuadObject>(1);
			m_pText[4]->Init();
			ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\textGameClear.png");
			m_pText[4]->GetComponent<MaterialComponent>()->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
			m_pText[4]->Transform()->SetPosition({ 0, 4.0f, - 9 });
			m_pText[4]->Transform()->SetScale({ 10, 10, 1 });
		}

		m_pSEgameclear->Play(false);
		break;

	case MODE::TO_TITLE:
		m_Mode = TO_TITLE;
		m_Timer = 0;

		// 破棄 -----
		m_pCannon->RequestDestroy();

		// フェード処理 -----
		m_pFadeQuad->Transform()->SetPosition({ 0, 5, -16 });
		dynamic_cast<FadeObject*>(m_pFadeQuad)->SetFade(FadeObject::Fade::Out);
		dynamic_cast<FadeObject*>(m_pFadeQuad)->SetFadeTime(2.0f);

		break;
	}
	m_NextMode = MODE::NONE;

	/*if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		Manager::SetScene<Result>();
	}*/
}



/*
ゲームメモ

連続キャッチ
金リンゴ

カゴのふちに掬すくいあげ

リンゴがぷにっと膨らむ＋エフェクト



*/