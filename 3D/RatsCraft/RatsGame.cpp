#include "includeCpp.h"
#include "RatsGame.h"

namespace RatsCraft
{
	RatsGame::RatsGame()
	{

	}

	RatsGame::~RatsGame()
	{

	}

	void RatsGame::Update(const float dt)
	{
		for (auto& obj : m_gameObjs)
		{
			obj.Update(dt);
		}
	}
}
