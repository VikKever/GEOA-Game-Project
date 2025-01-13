#include "Ball.h"
#include "utils.h"
#include <iostream>
#include "BoundingBox.h"
#include "GAUtils.h"
#include <algorithm>

Ball::Ball(const ThreeBlade& pos, const Motor& velocity, bool isProjectile)
	:m_pos{ pos }, m_velocity{ velocity }, m_isProjectile{ isProjectile }, m_isDead{ false }
{
}

void Ball::Draw() const
{
	const Ellipsef shape{ m_pos[0] / m_pos[3], m_pos[1] / m_pos[3], SIZE / 2, SIZE / 2};

	const float healthValue{ m_pos[2] / m_pos[3] };
	if (m_isProjectile)
	{
		utils::SetColor(Color4f{ 0.f, healthValue, 0.f, 1.f });
	}
	else
	{
		utils::SetColor(Color4f{ healthValue, 0.f, 0.f, 1.f });
	}
	utils::FillEllipse(shape);

	utils::SetColor(Color4f{ 1.f, 1.f, 1.f, 1.f });
	utils::DrawEllipse(shape);
}

void Ball::Update(float elapsedSec, const BoundingBox* boundingBox)
{
	Move(elapsedSec);
	CheckBoundingBoxCollision(boundingBox);

	// these values were slowly becoming invalid numbers, so set them back at their right values every frame
	// (otherwise there would be energy losses after a while)
	m_velocity[6] = 0.f;
	m_velocity[0] = 1.f;

	m_pos.Normalize();

	m_pos[2] = std::clamp(m_pos[2], 0.f, 1.f);
}

bool Ball::CheckCollision(Ball& other, bool changeOwnVelocity)
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
		if (changeOwnVelocity) m_velocity = projectedOtherVelocity * GAUtils::Scale(projectedVelocity, -1.f) * m_velocity;

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

		// If one of the two balls is a projectile, kill it
		if (m_isProjectile) Kill();
		if (other.m_isProjectile) other.Kill();

		return true;
	}
	return false;
}

void Ball::SetFlatVelocity(const Motor& translationMotor)
{
	float energyVelocity{ m_velocity[3] };
	m_velocity = translationMotor;
	m_velocity[3] = energyVelocity;
}

ThreeBlade Ball::GetFlatPos() const
{
	return ThreeBlade{ m_pos[0], m_pos[1], 0, 1 };
}

float Ball::GetEnergy() const
{
	return m_pos[2];
}

void Ball::AddEnergy(float amount)
{
	m_pos[2] += amount;
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
	//m_velocity = GAUtils::Scale(m_velocity, std::powf(FRICTION, elapsedSec));
}

void Ball::CheckBoundingBoxCollision(const BoundingBox* boundingBox, bool isFirstShot)
{
	OneBlade collisionPlane;
	// check whether the ball collides and move the position to be inside the box
	if (boundingBox->PlaceBackInsideBox(m_pos, SIZE / 2, collisionPlane))
	{
		// miror the velocity
		MultiVector transformedVelocity{ collisionPlane * m_velocity * ~collisionPlane };
		m_velocity = transformedVelocity.ToMotor();
	}
}
