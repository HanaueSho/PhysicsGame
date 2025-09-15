/*
	Title.h
	20250625  hanaue sho
*/
#include "title.h"
#include "manager.h"

#include "GameObject.h"
#include "polygon.h"
#include "camera.h"
#include "keyboard.h"
#include "game.h"
#include "audio.h"

#include "SkydomeObject.h"
#include "GroundObject.h"
#include "FadeObject.h"
#include "QuadObject.h"
#include "TreeObject.h"
#include "AppleSpawnerObject.h"
#include "CameraComponent.h"
#include "TransformComponent.h"
#include "BillboardComponent.h"

#include "manager.h"

void Title::Init()
{
	Scene::Init();

	// カメラ -----
	m_pCamera = AddGameObject<Camera>(0);
	m_pCamera->Init();
	m_pCamera->GetComponent<CameraComponent>()->SetMode(CameraComponent::Mode::Perspective);
	m_pCamera->GetComponent<CameraComponent>()->
		SetPerspective(DirectX::XMConvertToRadians(40.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f);
	m_pCamera->GetComponent<TransformComponent>()->SetPosition({ -20, 15, -10 });
	m_pCamera->GetComponent<TransformComponent>()->SetEulerAngles({ PI * -0.3f, PI * 0.2f, 0 });

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


	// フェード設定 -----
	m_pFadeQuad = AddGameObject<FadeObject>(1);
	m_pFadeQuad->Init();
	m_pFadeQuad->Transform()->SetScale({ 20, 20, 1 });
	m_pFadeQuad->Transform()->SetPosition({ -18, 21, -7 });
	dynamic_cast<FadeObject*>(m_pFadeQuad)->SetFade(FadeObject::Fade::In);
	dynamic_cast<FadeObject*>(m_pFadeQuad)->SetFadeTime(2.0f);
	m_pFadeQuad->GetComponent<BillboardComponent>()->SetType(BillboardComponent::BillboardType::Spherical);

	// タイトル -----
	m_pText[0] = AddGameObject<QuadObject>(1);
	m_pText[0]->Init();
	ID3D11ShaderResourceView* srv = Texture::Load("assets\\texture\\applePlop.png");
	m_pText[0]->GetComponent<MaterialComponent>()->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
	m_pText[0]->Transform()->SetPosition({ -17, 24, -6 });
	m_pText[1] = AddGameObject<QuadObject>(1);
	m_pText[1]->Init();
	srv = Texture::Load("assets\\texture\\textPushEnter.png");
	m_pText[1]->GetComponent<MaterialComponent>()->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);
	m_pText[1]->Transform()->SetPosition({ -16, 20.2, -5 });
	m_pText[1]->Transform()->SetScale({ 3, 3, 1 });

	// 木
	GameObject* pTree = AddGameObject<TreeObject>(1);
	pTree->Init();
	GameObject* pSpawner = AddGameObject<AppleSpawnerObject>(1);
	pSpawner->Init();
	pSpawner->Transform()->SetPosition({ -20, 80, 0 });
	pSpawner = AddGameObject<AppleSpawnerObject>(1);
	pSpawner->Init();
	pSpawner->Transform()->SetPosition({ -15, 80, 7 });
	pSpawner = AddGameObject<AppleSpawnerObject>(1);
	pSpawner->Init();
	pSpawner->Transform()->SetPosition({ -25, 80, 13 });
	pSpawner = AddGameObject<AppleSpawnerObject>(1);
	pSpawner->Init();
	pSpawner->Transform()->SetPosition({ -40, 80, 15 });
	pSpawner = AddGameObject<AppleSpawnerObject>(1);
	pSpawner->Init();
	pSpawner->Transform()->SetPosition({ -40, 80, 20 });
	pSpawner = AddGameObject<AppleSpawnerObject>(1);
	pSpawner->Init();
	pSpawner->Transform()->SetPosition({ -40, 80, 23 });


	// BGM再生
	m_pBGM = new Audio();
	m_pBGM->Load("assets\\audio\\BGMtitle.wav");
	m_pBGM->Play(true);
	m_pSEenter = new Audio();
	m_pSEenter->Load("assets\\audio\\enter.wav");
}

void Title::Uninit()
{
	// BGM停止
	m_pBGM->Uninit();
	delete m_pBGM;
	m_pSEenter->Uninit();
	delete m_pSEenter;

	Scene::Uninit();
}

void Title::Update(float dt)
{
	Scene::Update(dt);
	
	if (Keyboard_IsKeyDownTrigger(KK_ENTER) && !m_IsFade)
	{
		dynamic_cast<FadeObject*>(m_pFadeQuad)->SetFade(FadeObject::Fade::Out);
		m_IsFade = true;
		m_pSEenter->Play(false);
	}
	if (m_IsFade)
	{
		m_Timer += dt;
	}
	if (m_Timer > 3.0f)
	{
		Manager::SetScene<Game>();
	}

}
