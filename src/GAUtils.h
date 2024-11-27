#pragma once
#include "FlyFish.h"

namespace GAUtils
{
	inline bool IsBehindPlane(const ThreeBlade& point, const OneBlade& plane)
	{
		return (point & plane) < 0;
	}

	inline ThreeBlade Project(const ThreeBlade& point, const OneBlade& referencePlane)
	{
		return ((point | referencePlane) * referencePlane).Grade3();
	}
}