#pragma once
#include "FlyFish.h"

namespace GAUtils
{
	inline ThreeBlade Project(const ThreeBlade& point, const OneBlade& referencePlane)
	{
		return ((point | referencePlane) * referencePlane).Grade3();
	}

	inline Motor Reject(const Motor& motor, const TwoBlade& referenceLine)
	{
		return ((motor ^ referenceLine) * referenceLine);
	}

	/// <summary>
	/// Scale a translation with the given scale
	/// </summary>
	/// <param name="motor">The motor to scale</param>
	/// <param name="scale">The scale by which the motor gets multiplied</param>
	/// <returns></returns>
	inline Motor Scale(const Motor& motor, float scale)
	{
		Motor result = scale * motor;

		// set the scalar element to 1 so that the motor is still normalized
		// (otherwise the scaling wouldn't have an effect due to spacial equivalence)
		result[0] = 1.f;
		return result;
	}
}