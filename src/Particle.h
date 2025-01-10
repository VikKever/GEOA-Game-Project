#pragma once
#include "FlyFish.h"
#include "structs.h"
#include <vector>

class BoundingBox;

class Particle
{
public:
	Particle(const ThreeBlade& pos, const Motor& velocity);

	void Draw() const;
	void Update(float elapsedSec, const BoundingBox* boundingBox);
	void CheckParticleCollision(Particle& other);

	ThreeBlade GetPos() const;

	static constexpr float SIZE{ 30.f };

private:
	ThreeBlade m_pos;
	Motor m_velocity;

	void Move(float elapsedSec);
	void CheckBoundingBoxCollision(const BoundingBox* boundingBox);
};
