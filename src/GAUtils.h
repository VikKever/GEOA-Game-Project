#pragma once
#include "FlyFish.h"

namespace GAUtils
{
	inline ThreeBlade Project(const ThreeBlade& point, const OneBlade& referencePlane)
	{
		return ((point | referencePlane) * referencePlane).Grade3();
	}
}