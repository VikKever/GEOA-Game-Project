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
{
}

void Cue::Draw() const
{
	const float tipAngle{ 2.5f };

	const TwoBlade ballToCueLine{ (m_pWhiteBall->GetFlatPos() & m_cuePos) };
	
	const ThreeBlade cueBack1{ GetCueBackPoint(tipAngle, ballToCueLine) };
	const ThreeBlade cueBack2{ GetCueBackPoint(-tipAngle, ballToCueLine) };
	
	if (m_isShooting)
	{
		utils::SetColor(Color4f{ 0, 1, 0, 1 });
	}
	else
	{
		utils::SetColor(Color4f{ 1, 1, 0, 1 });
	}
	utils::FillTriangle(Point2f{ m_cuePos[0], m_cuePos[1] }, Point2f{ cueBack1[0], cueBack1[1] }, Point2f{ cueBack2[0], cueBack2[1] });
}

void Cue::Update(const Point2f& mousePosPt, bool isShooting)
{
	m_isShooting = isShooting;

	m_prevCuePos = m_cuePos;

	const ThreeBlade mousePos{ mousePosPt.x, mousePosPt.y, 0, 1 };

	if (m_isShooting)
	{
		// when shooting, project the mouse position to the line on which the player is shooting
		const TwoBlade shootLine{ m_cuePos & m_pWhiteBall->GetFlatPos() };
		m_cuePos = GAUtils::Project(mousePos, shootLine).Normalized();
	}
	else
	{
		m_cuePos = mousePos;
	}
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

ThreeBlade Cue::GetCueBackPoint(float angle, const TwoBlade& ballToCueLine) const
{
	const float cueLength{ 150.f };

	// rotate the line by the angle
	const Motor rotator{ Motor::Rotation(angle, TwoBlade{0, 0, 0, 0, 0, 1}) };
	const TwoBlade rotatedLine{ (rotator * ballToCueLine * ~rotator).Grade2() };

	// get the plane perpendicular to the line through the ball, used later as a vector to translate with
	const OneBlade offsetDirectionVector{ (rotatedLine | m_pWhiteBall->GetFlatPos()).Normalized() };

	// get the translation to translation the cuepoint
	const Motor translation{ GAUtils::TranslationFromOneBlade(offsetDirectionVector * cueLength) };
	return ThreeBlade{ (translation * m_cuePos * ~translation).Grade3().Normalized() };
}
