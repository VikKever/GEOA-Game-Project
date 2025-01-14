#include "Player.h"
#include "Ball.h"
#include "BoundingBox.h"
#include "utils.h"
#include "GAUtils.h"
#include <iostream>
#include "SDL_keyboard.h"
#include "Game.h"

Player::Player(const ThreeBlade& pos, const Motor& velocity, const ThreeBlade& pivotPos)
	: m_pivotPos{ pivotPos }
	, m_pos{ pos }
	, m_damageCountdown{ -1.f }
{
}

void Player::Update(float elapsedSec, const BoundingBox* pBoundingBox)
{
	bool lostEnergy{ false };

	const Uint8* pKeyState{};
	pKeyState = SDL_GetKeyboardState(nullptr);

	int rotationDir{-pKeyState[SDL_SCANCODE_A] + pKeyState[SDL_SCANCODE_D]};

	// ROTATION
	// ===================
	if (std::abs(rotationDir) > 0)
	{
		// Lose energy (do this if the player wants to rotate, even if there is no energy)
		// (this is done to make sure the player needs to stop pressing the rotate button for a while before it can start rotating again)
		const float energyLossOnRotation{ 0.4f };
		m_pos[2] -= energyLossOnRotation * elapsedSec;
		lostEnergy = true;

		// only rotate if there is enough energy
		if (m_pos[2] > FLT_EPSILON)
		{
			// rotate the ball by creating a force perpendicular to the line from the ball to the pivot
			// =================================
			const float rotationForce{ 200.f };

			// get pos without the z-axis
			const ThreeBlade flatPos{ m_pos[0], m_pos[1], 0, 1 };

			// get the line from the ball to the pivot
			const TwoBlade lineToPivot{ flatPos & m_pivotPos };
			// get a plane representing a translation vector
			// calculate the plane by taking the dot product between the line and a random point
			// scale by the rotation force and deltatime
			const OneBlade moveVector{ (lineToPivot | ThreeBlade{0, 0, 0}).Normalized() * rotationForce * elapsedSec };

			// the translation vector should be rotated around a line perpendicular to the playing field
			const TwoBlade rotateAxis{ 0, 0, 0, 0, 0, 1 };
			const Motor rotator{ Motor::Rotation(90 * rotationDir, rotateAxis) };
			const OneBlade rotatedMoveVector{ (rotator * moveVector * ~rotator).Grade1() };
			
			// offset the position with the vector
			const Motor velocity{ GAUtils::TranslationFromOneBlade(rotatedMoveVector) };
			m_pos = (velocity * m_pos * ~velocity).Grade3();
		}
	}

	// PIVOT MOVEMENT
	// =============
	const float pivotSpeed{ 200.f };

	Motor movement{ 1, 0, 0, 0, 0, 0, 0, 0 };

	if (pKeyState[SDL_SCANCODE_LEFT])
	{
		movement = movement * Motor::Translation(pivotSpeed * elapsedSec, TwoBlade{ -1.f, 0, 0, 0, 0, 0 });
	}
	if (pKeyState[SDL_SCANCODE_RIGHT])
	{
		movement = movement * Motor::Translation(pivotSpeed * elapsedSec, TwoBlade{ 1.f, 0, 0, 0, 0, 0 });
	}
	if (pKeyState[SDL_SCANCODE_DOWN])
	{
		movement = movement * Motor::Translation(pivotSpeed * elapsedSec, TwoBlade{ 0, -1.f, 0, 0, 0, 0 });
	}
	if (pKeyState[SDL_SCANCODE_UP])
	{
		movement = movement * Motor::Translation(pivotSpeed * elapsedSec, TwoBlade{ 0, 1.f, 0, 0, 0, 0 });
	}
	m_pivotPos = (movement * m_pivotPos * ~movement).Grade3();

	// BOUNDING BOX CHECK
	// ==================
	OneBlade collisionPlane{};
	pBoundingBox->PlaceBackInsideBox(m_pos, SIZE / 2, collisionPlane);

	// LOSE ENERGY
	// ================
	if (!lostEnergy)
	{
		const float energyGain{ 0.4f };
		m_pos[2] += energyGain * elapsedSec;
	}

	// DECREMENT DAMAGE COUNTDOWN
	// ====================
	m_damageCountdown -= elapsedSec;
}

void Player::Draw() const
{
	// draw player
	// ==========
	const Rectf shape{ m_pos[0] / m_pos[3] - SIZE / 2, m_pos[1] / m_pos[3] - SIZE / 2, SIZE, SIZE};

	// draw fill
	utils::SetColor(Color4f{ 0.f, 0.f, m_pos[2], 1.f}); // color depends on the energy
	utils::FillRect(shape);

	// draw outline
	if (m_damageCountdown <= 0.f)
	{
		utils::SetColor(Color4f{ 1, 1, 1, 1 });
	}
	else
	{
		// set color to red if damaged
		utils::SetColor(Color4f{ 1, 0, 0, 1 });
	}
	utils::DrawRect(shape);

	// draw pivot
	// ===========
	const float pivotSize{ 10.f };

	utils::SetColor(Color4f{ 1, 0, 1, 1 });
	utils::FillRect(m_pivotPos[0] - pivotSize / 2, m_pivotPos[1] - pivotSize / 2, pivotSize, pivotSize);
}

void Player::ReflectAroundPillar()
{
	if (m_pos[2] > MIN_REFLECT_ENERGY)
	{
		m_pos = (m_pivotPos * m_pos * ~m_pivotPos).Grade3();
		m_pos[2] = 0.f; // reflecting sets the energy back to 0
	}
}

bool Player::CheckHitsPlayer(const Ball& ball)
{
	if (m_damageCountdown > 0.f || ball.IsProjectile()) return false;

	const ThreeBlade flatPos{ m_pos[0], m_pos[1], 0, 1 };
	if ((ball.GetFlatPos() & flatPos).Norm() < (Ball::SIZE / 2 + SIZE / 2))
	{
		Game::AddLives(-1);
		m_damageCountdown = 2.f; // the player can't be hurt for 2 seconds
		m_pos[2] = 0.f; // remove energy
		return true;
	}
	return false;
}

std::unique_ptr<Ball> Player::ShootBall() const
{
	const float shootVelocity{ 5.f };
	OneBlade velocityVector{ (m_pos & m_pivotPos) | ThreeBlade{0, 0, 0} };
	velocityVector[3] = 0.f; // no energy gain/loss
	const Motor ballVelocity{ GAUtils::TranslationFromOneBlade(shootVelocity * velocityVector) };
	const ThreeBlade posWithEnergy{ m_pos[0], m_pos[1], 1.f, 1.f };
	return std::make_unique<Ball>(posWithEnergy, ballVelocity, true);
}
