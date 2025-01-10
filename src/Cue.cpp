#include "Cue.h"
#include "SDL_mouse.h"
#include "Ball.h"
#include "utils.h"
#include "GAUtils.h"

Cue::Cue(Ball* pWhiteBall)
	:m_pWhiteBall{ pWhiteBall }
	, m_cuePos{ 0, 0, 0, 1 }
	, m_prevCuePos{ 0, 0, 0, 1 }
	, m_isShooting{ false }
	, m_shootOffset{ 0.f }
{
}

void Cue::Draw() const
{
	const float tipAngle{ 2.f }; // not actually the angle but proportional to it
	const float cueLength{ 200.f };

	const TwoBlade ballToCueLine{ (m_pWhiteBall->GetFlatPos() & m_cuePos) };
	
	const ThreeBlade cueBack1{ MovePointAlongLine(m_cuePos, cueLength, ballToCueLine, tipAngle) };
	const ThreeBlade cueBack2{ MovePointAlongLine(m_cuePos, cueLength, ballToCueLine, -tipAngle) };
	
	if (m_isShooting)
	{
		utils::SetColor(Color4f{ 0.8, 0.4f, 0, 1 });
	}
	else
	{
		utils::SetColor(Color4f{ 0.18f, 0.1f, 0, 1 });
	}
	utils::FillTriangle(Point2f{ m_cuePos[0], m_cuePos[1] }, Point2f{ cueBack1[0], cueBack1[1] }, Point2f{ cueBack2[0], cueBack2[1] });
}

void Cue::Update(const Point2f& mousePosPt, bool isShooting)
{
	m_isShooting = isShooting;

	m_prevCuePos = m_cuePos;

	const ThreeBlade mousePos{ mousePosPt.x, mousePosPt.y, 0, 1 };

	ThreeBlade cueMiddlePos{};
	if (m_isShooting)
	{
		// when shooting, project the mouse position to the line on which the player is shooting
		const TwoBlade projectLine{ (m_cuePos & m_pWhiteBall->GetFlatPos()).Normalized() };
		cueMiddlePos = GAUtils::Project(mousePos, projectLine).Normalized();
	}
	else
	{
		cueMiddlePos = mousePos;
	}

	const TwoBlade offsetLine{ (cueMiddlePos & m_pWhiteBall->GetFlatPos()) };

	float moveDst{ 150.f };
	if (!m_isShooting)
	{
		// make sure the cue can't intersect with the ball
		const float dstToBall{ offsetLine.Norm() - Ball::SIZE / 2 };
		const float minDst{ 10.f };
		if (moveDst > dstToBall - minDst)
		{
			m_shootOffset = moveDst - dstToBall + minDst;
			moveDst = dstToBall - minDst;
		}
	}
	else
	{
		moveDst -= m_shootOffset;
	}

	m_cuePos = MovePointAlongLine(cueMiddlePos, moveDst, offsetLine);
}

bool Cue::CheckHitBall()
{
	// check if the cue intersects the ball
	const float dstToBall{ (m_cuePos & m_pWhiteBall->GetFlatPos()).Norm() };
	if (dstToBall < Ball::SIZE / 2)
	{
		const float forceMultiplier{ 50.f };

		// get the line on which the cue is moving
		const TwoBlade moveLine{ m_prevCuePos & m_cuePos };
		// get a plane perpendicular to the line through the point, counting as a translation vector
		const OneBlade moveVector{ moveLine | m_pWhiteBall->GetFlatPos() };
		// get the final translation with this vector oneblade
		const Motor translation{ GAUtils::TranslationFromOneBlade(moveVector) };

		m_pWhiteBall->ApplyForce(translation * forceMultiplier);

		return true;
	}
	return false;
}

ThreeBlade Cue::MovePointAlongLine(const ThreeBlade& point, float distance, const TwoBlade& referenceLine, float angle)
{
	// rotate the line by the angle
	const Motor rotator{ Motor::Rotation(angle, TwoBlade{0, 0, 0, 0, 0, 1}) };
	const TwoBlade rotatedLine{ (rotator * referenceLine * ~rotator).Grade2() };

	// get a plane perpendicular to the line, used later as a vector to translate with
	const OneBlade offsetDirectionVector{ (rotatedLine | ThreeBlade{0, 0, 0, 1}).Normalized()};

	// get the translation to translation the point
	const Motor translation{ GAUtils::TranslationFromOneBlade(offsetDirectionVector * distance) };
	return (translation * point * ~translation).Grade3();
}
