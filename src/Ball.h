#pragma once
#include "FlyFish.h"
#include "structs.h"
#include <vector>

class BoundingBox;

class Ball
{
public:
	Ball(const ThreeBlade& pos, const Motor& velocity, bool isProjectile);

	void Draw() const;
	void Update(float elapsedSec, const BoundingBox* boundingBox);
	bool CheckCollision(Ball& other, bool changeOwnVelocity = true);

	void SetFlatVelocity(const Motor& translationMotor);
	ThreeBlade GetPos() const { return m_pos; }
	void SetPos(const ThreeBlade& newPos) { m_pos = newPos; }
	ThreeBlade GetFlatPos() const;
	float GetEnergy() const;
	void AddEnergy(float amount);
	bool IsProjectile() const { return m_isProjectile; }
	void Kill() { m_isDead = true; }
	bool IsDead() const { return m_isDead; }

	static constexpr float SIZE{ 30.f };

private:
	bool m_isProjectile;
	ThreeBlade m_pos;
	Motor m_velocity;

	bool m_isDead;

	void Move(float elapsedSec);
	void CheckBoundingBoxCollision(const BoundingBox* boundingBox, bool isFirstShot = false);
};
