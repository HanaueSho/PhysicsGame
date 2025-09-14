/*
	Game.h
	20250625  hanaue sho
*/
#ifndef GAME_H_
#define GAME_H_
#include "scene.h"

class Game : public Scene
{
private:
	class Audio* m_pBGM;

	enum MODE
	{
		NONE,
		START,
		COLLECTION,
		TO_SHOT,
		SHOT,
		GAMEOVER
	};
	MODE m_Mode = MODE::NONE;
	MODE m_NextMode = MODE::NONE;

	// ���Ԍo��
	float m_TimerColl = 0.0f;
	float m_TimeColl = 10.0f;

	// �Q�[���I�u�W�F�N�g
	GameObject* m_pPlayer  = nullptr; // �v���C���[
	GameObject* m_pSlop	   = nullptr; // �⓹
	GameObject* m_pSpawner = nullptr; // �X�|�i�[
	GameObject* m_pCannon  = nullptr; // ��C
	GameObject* m_pTower   = nullptr; // �^���[


public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

	void SetGameMode(MODE m) { m_NextMode = m; }
};





#endif