#pragma once

#include "include.h"

#include <combaseapi.h>

#include "EventType.h"

namespace TetrisSpace
{
	interface EventListener
	{
	public:
		virtual ~EventListener() {};

		virtual void HandleEvent(EventType type, void* pEventData) = 0;
	};
}