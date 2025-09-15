/*
	FadeObject.h
	20250915 hanaue sho
	平面のオブジェクト
*/
#ifndef FADEOBJECT_H_
#define FADEOBJECT_H_
#include "GameObject.h"

class FadeObject : public GameObject
{
public:
	enum class Fade { None, In, Out};

private:
	Fade m_FadeMode = Fade::None;
	float m_FadeTime = 1.0f;
	float m_Timer = 0.0f;

public:
	void Init() override;
	void Update(float dt) override;

	void SetFade(Fade f) { m_FadeMode = f; }
	void SetFadeTime(float t) { m_FadeTime = t; }
};

#endif