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
	class Audio* m_pSEgameclear;
	class Audio* m_pSEgameover;
	class Audio* m_pSEtimeup;
	class Audio* m_pSEenter;

	enum MODE
	{
		NONE,
		START,
		COLLECTION,
		TO_SHOT_1,
		TO_SHOT_2,
		SHOT,
		GAMEOVER,
		GAMECLEAR,
		TO_TITLE
	};
	MODE m_Mode = MODE::NONE;
	MODE m_NextMode = MODE::NONE;

	// ���Ԍo��
	float m_Timer = 0.0f;
	float m_TimeColl = 60.0f;
	float m_TimeToShot = 2.5f;
	float m_Interval = 3.0f;

	// �Q�[���I�u�W�F�N�g
	GameObject* m_pCamera     = nullptr; // �J����
	GameObject* m_pPlayer     = nullptr; // �v���C���[
	GameObject* m_pSlop	      = nullptr; // �⓹
	GameObject* m_pSpawner[2]{};		 // �X�|�i�[
	GameObject* m_pCannon     = nullptr; // ��C
	GameObject* m_pTower_1	  = nullptr; // �^���[
	GameObject* m_pTower_2	  = nullptr; // �^���[
	GameObject* m_pTower_3	  = nullptr; // �^���[

	GameObject* m_pFadeQuad = nullptr;
	GameObject* m_pText[5]{}; // �e�L�X�g�z��
	/*
	0 : �Q�[���X�^�[�g
	1 : ��������i�J�S�j
	2 : ��������i��C�j
	3 : �Q�[���I�[�o�[
	4 : �Q�[���N���A
	*/


public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

	void SetGameMode(MODE m) { m_NextMode = m; }
};





#endif