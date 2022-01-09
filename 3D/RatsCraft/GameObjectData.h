#pragma once

#include "include.h"

namespace RatsCraft
{
	enum class GameObjectType
	{
		NONE,
		TESTTYPE_BOX,
		TESTTYPE_TRIANGLE
	};

	struct RatsCraftAPI GameObjectData
	{
		GameObjectData() = default;
		~GameObjectData() = default;

		float speed = 1;
		GameObjectType type = GameObjectType::NONE;
	};
}