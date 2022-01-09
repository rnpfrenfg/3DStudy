#pragma once

#include "EventType.h"

namespace RatsCraft
{
	class RatsCraftAPI EventListener
	{
	public:
		virtual ~EventListener() {};

		virtual void HandleEvent(EventType type, void* pEventData) = 0;
	};
}