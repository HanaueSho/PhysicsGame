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


void Title::Init()
{
	Scene::Init();

	Camera* pCamera = AddGameObject<Camera>(0);
	pCamera->Init();
	Polygon2D* pPolygon = AddGameObject<Polygon2D>(2);
	pPolygon->Init();

	// BGMÄ¶
	m_pBGM = new Audio();
	m_pBGM->Load("assets\\audio\\BGMtitle.wav");
	//m_pBGM->Play(true);
}

void Title::Uninit()
{
	// BGM’âŽ~
	m_pBGM->Uninit();
	delete m_pBGM;

	Scene::Uninit();
}

void Title::Update(float dt)
{
	Scene::Update(dt);
	if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		Manager::SetScene<Game>();
	}
}
