#pragma once

#include "include.h"

#include "Location.h"
#include "GameObjectData.h"

namespace RatsCraft
{
	class RatsCraftAPI GameObject
	{
	public:
		GameObject();
		~GameObject();

		void Init(GameObjectData* data);

		void Update(const float dt);

		Location forward = Location(0, 0);

		GameObjectData* m_objData = nullptr;
		Location loc;
	private:

	};
}
