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

public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

};





#endif