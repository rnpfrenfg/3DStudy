#pragma once

#include "include.h"

#include <math.h>

namespace RatsCraft
{
	class RatsCraftAPI Location
	{
	public:
		Location() = default;
		Location(float x, float y);

		void operator+=(const Location& loc);

		void operator-=(const Location& loc);

		float Size();

		void Normalize();

		float x = 0;
		float y = 0;
		float z = 0;
	};
}

