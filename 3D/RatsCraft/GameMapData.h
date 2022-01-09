#pragma once

#include "Tile.h"

namespace RatsCraft
{
	enum { MAX_TILES_IN_MAP = 10000 };
	enum { MAP_LENGTH = 1000 };

	struct RatsCraftAPI GameMapData
	{
	public:
		Tile tileData[MAX_TILES_IN_MAP];
		int tiles[MAP_LENGTH][MAP_LENGTH];


	};
}