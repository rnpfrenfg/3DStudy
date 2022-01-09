#include "includeCpp.h"
#include "Location.h"

namespace RatsCraft
{
	Location::Location(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	void Location::operator+=(const Location& loc)
	{
		this->x += loc.x;
		this->y += loc.y;
		this->z += loc.z;
	}

	void Location::operator-=(const Location& loc)
	{
		this->x -= loc.x;
		this->y -= loc.y;
		this->z -= loc.z;
	}

	float Location::Size()
	{
		float sum = x * x + y * y + z * z;
		return sqrt(sum);
	}

	void Location::Normalize()
	{
		float size = Size();

		x /= size;
		y /= size;
		z /= size;
		return;
	}
}