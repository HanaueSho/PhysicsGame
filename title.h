/*
	Title.h
	20250625  hanaue sho
*/
#ifndef TITLE_H_
#define TITLE_H_
#include "scene.h"


class Title : public Scene
{
private:
	GameObject* m_pCamera = nullptr; // カメラ
	GameObject* m_pFadeQuad = nullptr;
	GameObject* m_pText[2]{}; // テキスト配列
	bool m_IsFade = false;
	float m_Timer = 0.0f;

	class Audio* m_pBGM;
	class Audio* m_pSEenter;



public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;
};





#endif