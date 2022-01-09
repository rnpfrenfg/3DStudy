#pragma once

#include "include.h"

namespace RatsCraft
{
	enum class RatsCraftAPI EventType
	{
		TEST,

		NUM_EVENTS
	};

	enum { CNUM_EVENTS = static_cast<int>(EventType::NUM_EVENTS) };
}