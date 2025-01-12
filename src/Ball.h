#pragma once
#include "FlyFish.h"
#include "structs.h"
#include <vector>

class BoundingBox;

class Ball
{
public:
	Ball(const ThreeBlade& pos, const Motor& velocity);

	void Draw() const;
	void Update(float elapsedSec, const BoundingBox* boundingBox);
	bool CheckParticleCollision(Ball& other);

	void ApplyForce(const Motor& translationMotor);
	//ThreeBlade GetPos() const;
	ThreeBlade GetFlatPos() const;
	int GetPoints() const;

	static constexpr float SIZE{ 30.f };

private:
	static constexpr int TOT_LIVES{ 20 };

	ThreeBlade m_pos;
	Motor m_velocity;

	void Move(float elapsedSec);
	void CheckBoundingBoxCollision(const BoundingBox* boundingBox, bool isFirstShot = false);
};
