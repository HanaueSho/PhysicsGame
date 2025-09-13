#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "scene.h"
#include "game.h"
#include "title.h"
#include "audio.h"

Scene* Manager::m_pScene = nullptr;
Scene* Manager::m_pSceneNext = nullptr;

void Manager::Init()
{
	Renderer::Init();
	Audio::InitMaster();

	//m_pScene = new Game();
	m_pScene = new Title();
	m_pScene->Init();

}


void Manager::Uninit()
{
	m_pScene->Uninit();
	delete m_pScene;

	Audio::UninitMaster();
	Renderer::Uninit();
}

void Manager::Update(float dt)
{
	m_pScene->Update(dt);
}

void Manager::FixedUpdate(float dt)
{
	m_pScene->FixedUpdate(dt);
}

void Manager::Draw()
{
	Renderer::Begin();

	m_pScene->Draw();

	Renderer::End();

	// ƒV[ƒ“Ø‚è‘Ö‚¦
	if (m_pSceneNext != nullptr)
	{
		m_pScene->Uninit();
		delete m_pScene;

		m_pScene = m_pSceneNext;
		m_pScene->Init();

		m_pSceneNext = nullptr;
	}
}
