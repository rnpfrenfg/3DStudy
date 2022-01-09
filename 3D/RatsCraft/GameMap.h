#pragma once

#include "include.h"

#include "GameMapData.h"
#include "GameObject.h"

namespace RatsCraft
{
	class RatsCraftAPI GameMap
	{
	public:
		GameMap() = default;
		~GameMap() = default;

		//void Init(GameMapData* mapData);

	private:
		GameMapData mapData;
	};
}