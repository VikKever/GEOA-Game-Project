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

	ThreeBlade GetCueBackPoint(float angle, const TwoBlade& ballToCueLine) const;
};