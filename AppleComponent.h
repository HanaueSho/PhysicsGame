/*
	AppleComponent.h
	20250915  hanaue sho
*/
#ifndef APPLECOMPONENT_H_
#define APPLECOMPONENT_H_
#include "Component.h"
#include "SlopComponent.h"
#include "BlockComponent.h"
#include "Audio.h"

class AppleComponent : public Component
{
private:
	Audio* m_pSEblock = nullptr;
	Audio* m_pSEslop = nullptr;
public:
	void Init() override
	{
		m_pSEblock = new Audio();
		m_pSEblock->Load("assets\\audio\\hitBlock.wav");
		m_pSEslop = new Audio();
		m_pSEslop->Load("assets\\audio\\appleSound.wav");
	}

	void OnCollisionEnter(class Collider* me, class Collider* other)override
	{
		if (other->Owner()->GetComponent<SlopComponent>())
		{
			m_pSEslop->Play(false);
		}
		if (other->Owner()->GetComponent<BlockComponent>())
		{
			m_pSEblock->Play(false);
		}
	}

};

#endif