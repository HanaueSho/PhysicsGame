/*
	BlockComponent.h
	20250915  hanaue sho
*/
#ifndef BLOCKCOMPONENT_H_
#define BLOCKCOMPONENT_H_
#include "Component.h"

class BlockComponent : public Component
{
private:
	bool m_IsCounted = false;

public:
	void Update(float dt) override
	{
		if (IsCounted()) return;

		if (Owner()->Transform()->Position().y < 2) // ‚O‚æ‚è‰º‚É‚¢‚Á‚½‚ç
			Count();
	}

	void Count() { m_IsCounted = true; }
	bool IsCounted() const { return m_IsCounted; }

};


#endif