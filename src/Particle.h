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
private:
	static constexpr float SIZE{ 20.f };

	ThreeBlade m_pos;
	Motor m_velocity;
	Motor m_rotation;
};
