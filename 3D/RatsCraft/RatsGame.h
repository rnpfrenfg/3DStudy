#pragma once

#include "include.h"

#include <vector>

#include "GameMap.h"
#include "GameMapData.h"
#include "EventListener.h"
#include "EventManager.h"
#include "GameObject.h"
#include "GameObjectData.h"

namespace RatsCraft
{
	class RatsCraftAPI RatsGame
	{
	public:
		RatsGame();
		~RatsGame();

		void Update(const float dt);

		std::vector<GameObject> m_gameObjs;

	};
}

