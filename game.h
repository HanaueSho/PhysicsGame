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

	// 時間経過
	float m_Timer = 0.0f;
	float m_TimeColl = 60.0f;
	float m_TimeToShot = 2.5f;
	float m_Interval = 3.0f;

	// ゲームオブジェクト
	GameObject* m_pCamera     = nullptr; // カメラ
	GameObject* m_pPlayer     = nullptr; // プレイヤー
	GameObject* m_pSlop	      = nullptr; // 坂道
	GameObject* m_pSpawner[2]{};		 // スポナー
	GameObject* m_pCannon     = nullptr; // 大砲
	GameObject* m_pTower_1	  = nullptr; // タワー
	GameObject* m_pTower_2	  = nullptr; // タワー
	GameObject* m_pTower_3	  = nullptr; // タワー

	GameObject* m_pFadeQuad = nullptr;
	GameObject* m_pText[5]{}; // テキスト配列
	/*
	0 : ゲームスタート
	1 : 操作説明（カゴ）
	2 : 操作説明（大砲）
	3 : ゲームオーバー
	4 : ゲームクリア
	*/


public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

	void SetGameMode(MODE m) { m_NextMode = m; }
};





#endif