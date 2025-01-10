#pragma once
#include "FlyFish.h"

class Ball;

class Hole
{
public:
	explicit Hole(const ThreeBlade& position);

	void Draw() const;
	bool FallsIn(const Ball& ball) const;
private:
	static constexpr float SIZE{ 30.f };

	ThreeBlade m_pos;
};