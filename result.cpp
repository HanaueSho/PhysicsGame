/*
	Result.h
	20250625  hanaue sho
*/
#include "result.h"
#include "manager.h"

#include "GameObject.h"
#include "polygon.h"
#include "camera.h"
#include "keyboard.h"
#include "title.h"
#include "audio.h"



void Result::Init()
{
	Camera* pCamera = AddGameObject<Camera>(0);

	float scale = 0.4f;
	Polygon2D* pPolygon = AddGameObject<Polygon2D>(2);
	pPolygon->Init();
	//pPolygon->Init({ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0 }, { 0, 0, 0 }, {1, 1, 1});

	// BGMÄ¶
	m_pBGM = new Audio();
	m_pBGM->Load("assets\\audio\\BGMresult.wav");
	m_pBGM->Play(true);
}

void Result::Uninit()
{
	// BGM’âŽ~
	m_pBGM->Uninit();
	delete m_pBGM;

	Scene::Uninit();
}

void Result::Update(float dt)
{
	Scene::Update(dt);
	if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		Manager::SetScene<Title>();
	}
}
