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

	// 時間経過
	float m_TimerColl = 0.0f;
	float m_TimeColl = 10.0f;

	// ゲームオブジェクト
	GameObject* m_pPlayer  = nullptr; // プレイヤー
	GameObject* m_pSlop	   = nullptr; // 坂道
	GameObject* m_pSpawner = nullptr; // スポナー
	GameObject* m_pCannon  = nullptr; // 大砲
	GameObject* m_pTower   = nullptr; // タワー


public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

	void SetGameMode(MODE m) { m_NextMode = m; }
};





#endif