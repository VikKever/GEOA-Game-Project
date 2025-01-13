#pragma once
#include "FlyFish.h"

namespace GAUtils
{
	inline ThreeBlade Project(const ThreeBlade& point, const OneBlade& referencePlane)
	{
		return ((point | referencePlane) * referencePlane).Grade3();
	}

	inline TwoBlade Project(const TwoBlade& line, const ThreeBlade& referencePoint)
	{
		return ((line | referencePoint) * referencePoint).Grade2();
	}

	inline ThreeBlade Project(const ThreeBlade& point, const TwoBlade& referenceLine)
	{
		return ((point | referenceLine) * referenceLine).Grade3();
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

	inline Motor TranslationFromOneBlade(const OneBlade& translation)
	{
		OneBlade e0{ 1, 0, 0, 0 };
		Motor oneMotor{ 1, 0, 0, 0, 0, 0, 0, 0 };

		return oneMotor - 0.5f * translation * (-e0);
	}

	inline Motor RotateAroundLine(float angle, const TwoBlade& line)
	{
		const TwoBlade linePart{ -(std::sinf(angle * DEG_TO_RAD / 2) * line) };
		return Motor{ std::cosf(angle * DEG_TO_RAD / 2), linePart[0], linePart[1], linePart[2], linePart[3], linePart[4], linePart[5], 0 };
	}
}