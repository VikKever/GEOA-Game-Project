#pragma once
#include "FlyFish.h"
#include "structs.h"

class BoundingBox;

class Particle
{
public:
	Particle(const ThreeBlade& pos, const Motor& velocity, const Motor& rotation);

	void Draw() const;
	void Update(float elapsedSec, const BoundingBox* boundingBox);

	ThreeBlade GetPos() const;

	static constexpr float SIZE{ 10.f };

private:
	ThreeBlade m_pos;
	Motor m_velocity;
	Motor m_rotation;
};
