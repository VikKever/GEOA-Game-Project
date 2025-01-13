#pragma once
#include "structs.h"
#include "FlyFish.h"

class BoundingBox
{
public:
	BoundingBox(const Rectf& box);

	bool Collides(const ThreeBlade& point, OneBlade& collision, float offset) const;
	bool PlaceBackInsideBox(ThreeBlade& point, float offset, OneBlade& collisionPlane) const;
private:
	OneBlade m_leftPlane;
	OneBlade m_rightPlane;
	OneBlade m_topPlane;
	OneBlade m_bottomPlane;
};