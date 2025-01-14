#pragma once
#include "FlyFish.h"

class Ball;

class Hole
{
public:
	explicit Hole(const ThreeBlade& position);

	void Draw() const;
	bool CheckFallsIn(Ball& ball) const;
private:
	static constexpr float SIZE{ 100.f };

	ThreeBlade m_pos;
};