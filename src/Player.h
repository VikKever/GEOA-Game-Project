#pragma once
#include "FlyFish.h"
#include <memory>

class Ball;
class BoundingBox;

class Player
{
public:
	Player(const ThreeBlade& pos, const Motor& velocity, const ThreeBlade& pivotPos);

	void Update(float elapsedSec, const BoundingBox* pBoundingBox);
	void Draw() const;
	void ReflectAroundPillar();
	bool CheckHitsPlayer(const Ball& ball);

	std::unique_ptr<Ball> ShootBall() const;
private:
	static constexpr float SIZE{ 25.f };
	static constexpr float MIN_ROTATE_ENERGY{ 0.05f };
	static constexpr float MIN_REFLECT_ENERGY{ 0.9f };

	ThreeBlade m_pos;
	ThreeBlade m_pivotPos;

	float m_damageCountdown;
};