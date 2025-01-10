#pragma once
#include "FlyFish.h"
#include "structs.h"
#include <vector>

class BoundingBox;

class Ball
{
public:
	Ball(const ThreeBlade& pos, const Motor& velocity, bool isWhite = false);

	void Draw() const;
	void Update(float elapsedSec, const BoundingBox* boundingBox, bool isFirstShot = false);
	void CheckParticleCollision(Ball& other, bool isFirstShot = false);

	void ApplyForce(const Motor& translationMotor);
	bool IsMoving() const;
	ThreeBlade GetPos() const;
	ThreeBlade GetFlatPos() const;

	static constexpr float SIZE{ 30.f };

private:
	static constexpr int TOT_LIVES{ 20 };
	static constexpr float FRICTION{ 0.6f };
	static constexpr float MIN_SPEED{ 2.f };

	ThreeBlade m_pos;
	Motor m_velocity;
	bool m_isWhiteBall;

	void Move(float elapsedSec);
	void CheckBoundingBoxCollision(const BoundingBox* boundingBox, bool isFirstShot = false);
};
