#pragma once
#include "FlyFish.h"
#include <memory>
#include "utils.h"

class Ball;

class Cue
{
public:
	explicit Cue(Ball* whiteBall);

	void Draw() const;
	void Update(const Point2f& mousePos, bool isShooting);
	bool CheckHitBall();
private:
	Ball* m_pWhiteBall;

	ThreeBlade m_prevCuePos;
	ThreeBlade m_cuePos;

	bool m_isShooting;
	float m_shootOffset;

	static ThreeBlade MovePointAlongLine(const ThreeBlade& point, float distance, const TwoBlade& referenceLine, float angle = 0.f);
};