#include "Ball.h"
#include "utils.h"
#include <iostream>
#include "BoundingBox.h"
#include "GAUtils.h"
#include <algorithm>

Ball::Ball(const ThreeBlade& pos, const Motor& velocity, bool isWhite)
	:m_pos{ pos }, m_velocity{ velocity }, m_isWhiteBall{ isWhite }
{
	m_pos[2] = float(TOT_LIVES);
}

void Ball::Draw() const
{
	const Ellipsef shape{ m_pos[0] / m_pos[3], m_pos[1] / m_pos[3], SIZE / 2, SIZE / 2};

	if (m_isWhiteBall)
	{
		utils::SetColor(Color4f{ 1.f, 1.f, 1.f, 1.f });
	}
	else
	{
		const float healthValue{ m_pos[2] / m_pos[3] / TOT_LIVES };
		utils::SetColor(Color4f{ healthValue * 0.5f + 0.5f, (1.f - healthValue) * 0.3f, (1.f - healthValue) * 0.2f, 1.f });
	}
	utils::FillEllipse(shape);

	utils::SetColor(Color4f{ 1.f, 1.f, 1.f, 1.f });
	//utils::DrawEllipse(shape);
}

void Ball::Update(float elapsedSec, const BoundingBox* boundingBox, bool isFirstShot)
{
	Move(elapsedSec);
	CheckBoundingBoxCollision(boundingBox);

	// these values were slowly becoming invalid numbers, so set them back at their right values every frame
	// (otherwise there would be energy losses after a while)
	m_velocity[6] = 0.f;
	m_velocity[0] = 1.f;

	m_pos.Normalize();
}

void Ball::CheckParticleCollision(Ball& other, bool isFirstShot)
{
	// ignore the z-axis:
	const ThreeBlade thisPos2D{ m_pos[0], m_pos[1], 0, 1 };
	const ThreeBlade otherPos2D{ other.m_pos[0], other.m_pos[1], 0, 1 };

	const TwoBlade rawJoinLine{ otherPos2D & thisPos2D };
	const float distance{ rawJoinLine.Norm() };
	const TwoBlade joinLine{ rawJoinLine.Normalized() };

	// check if there is a collision
	if (distance < SIZE)
	{
		//// calculate energy for debugging
		//const float beforeEnergy1{ std::powf(m_velocity.VNorm() * 2, 2) };
		//const float beforeEnergy2{ std::powf(other.m_velocity.VNorm() * 2, 2) };

		// Calculate the new velocities
		// ============================

		// project velocity to be in the direction of the joinline (=> vanishing line is perpendicular to the line)
		// reverse because the scalar value is negative
		const Motor projectedVelocity{ -GAUtils::Reject(m_velocity, joinLine) };
		const Motor projectedOtherVelocity{ -GAUtils::Reject(other.m_velocity, joinLine) };

		// Add the inverted force of the other ball and the force of this ball to the current velocity
		other.m_velocity = projectedVelocity * GAUtils::Scale(projectedOtherVelocity, -1.f) * other.m_velocity;
		m_velocity = projectedOtherVelocity * GAUtils::Scale(projectedVelocity, -1.f) * m_velocity;

		// Place the particles outside of eachother again
		// ==========================================
		
		// Calculate the vector to offset the balls by taking the dot between the joinLine and the origin
		// The result of this is an origin plane with a normal orthogonal to the line
		// Multiplied by 2 to make it normalized
		OneBlade offsetVector{ (joinLine | ThreeBlade{0, 0, 0, 1}) * 2 };
		offsetVector = offsetVector / 2;

		// calculate translation (add a small offset to avoid bouncing two times)
		const float translationAmount{ (SIZE - distance) / 2 + 0.01f };

		// Create offset motor and translate the particles
		const Motor thisOffset{ GAUtils::TranslationFromOneBlade(translationAmount * offsetVector)};
		m_pos = (thisOffset * m_pos * ~thisOffset).Grade3();

		const Motor otherOffset{ GAUtils::TranslationFromOneBlade(-translationAmount * offsetVector)};
		other.m_pos = (otherOffset * other.m_pos * ~otherOffset).Grade3();

		//// calculate energy after (debug)
		//const float afterEnergy1{ std::powf(m_velocity.VNorm() * 2, 2) };
		//const float afterEnergy2{ std::powf(other.m_velocity.VNorm() * 2, 2) };
		//const float energyDifference{ (afterEnergy1 + afterEnergy2) - (beforeEnergy1 + beforeEnergy2) };
		//if (energyDifference > 10.f || energyDifference < -10.f)
		//{
		//	std::cout << "Energy difference! (" << energyDifference << " units)\n";
		//}

		// Remove 1 live for collisions with a red ball, and 3 for collisions with the white ball
		// ============================
		if (!isFirstShot)
		{
			if (m_isWhiteBall) other.m_pos[2] -= 3.f;
			else
			{
				if (other.m_isWhiteBall) m_pos[2] -= 3.f;
				else
				{
					m_pos[2] -= 1.f;
					other.m_pos[2] -= 1.f;
				}
			}
		}
	}
}

void Ball::ApplyForce(const Motor& translationMotor)
{
	m_velocity = translationMotor * m_velocity;
}

bool Ball::IsMoving() const
{
	return m_velocity.VNorm() > MIN_SPEED;
}

ThreeBlade Ball::GetPos() const
{
	return m_pos;
}

ThreeBlade Ball::GetFlatPos() const
{
	return ThreeBlade{ m_pos[0], m_pos[1], 0, 1 };;
}

void Ball::Move(float elapsedSec)
{
	// Calculate movement
	// =================
	Motor totMotor{ m_velocity * elapsedSec };
	totMotor[0] = 1.f; // manually set the norm back to one (so only the translation part is multiplied by elapsedSec)

	// translate the particle with the velocity
	m_pos = (totMotor * m_pos * ~totMotor).Grade3();

	// Add friction by multiplying by elapsedSec and resetting the norm
	m_velocity = GAUtils::Scale(m_velocity, std::powf(FRICTION, elapsedSec));
}

void Ball::CheckBoundingBoxCollision(const BoundingBox* boundingBox, bool isFirstShot)
{
	OneBlade collisionPlane;
	// check whether the ball collides and save the plane it hit when it did
	if (boundingBox->Collides(m_pos, collisionPlane, SIZE / 2))
	{
		// project the position onto the plane
		m_pos = GAUtils::Project(m_pos, collisionPlane);
		// offset the ball with its radius
		Motor offset{ GAUtils::TranslationFromOneBlade(-(SIZE / 2) * collisionPlane) };
		m_pos = (offset * m_pos * ~offset).Grade3();

		// miror the velocity
		MultiVector transformedVelocity{ collisionPlane * m_velocity * ~collisionPlane };
		m_velocity = transformedVelocity.ToMotor();

		// Remove a life when bouncing against a wall
		if (!m_isWhiteBall && !isFirstShot) m_pos[2] -= 1.f;
	}
}
