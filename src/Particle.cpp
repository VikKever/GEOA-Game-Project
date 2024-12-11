#include "Particle.h"
#include "utils.h"
#include <iostream>
#include "BoundingBox.h"
#include "GAUtils.h"

Particle::Particle(const ThreeBlade& pos, const Motor& velocity, const Motor& rotation)
	:m_pos{ pos }, m_velocity{ velocity }, m_rotation{ rotation }
{
}

void Particle::Draw() const
{
	const Ellipsef shape{ m_pos[0] / m_pos[3], m_pos[1] / m_pos[3], SIZE / 2, SIZE / 2};

	const float energy{ m_pos[2] / m_pos[3]};
	utils::SetColor(Color4f{ energy, energy, energy, 1.f });
	utils::FillEllipse(shape);

	utils::SetColor(Color4f{ 1.f, 1.f, 1.f, 1.f });
	utils::DrawEllipse(shape);
}

void Particle::Update(float elapsedSec, const BoundingBox* boundingBox)
{
	Motor totMotor{ m_velocity * elapsedSec };
	totMotor[0] = 1.f; // manually set the norm back to one (so only the translation part is multiplied by elapsedSec)
	
	m_pos = (totMotor * m_pos * ~totMotor).Grade3();

	OneBlade collisionPlane;
	if (boundingBox->Collides(m_pos, collisionPlane, SIZE))
	{
		std::cout << "Bounce!" << std::endl;
		//OneBlade e0{ 1, 0, 0, 0 };
		//Motor oneMotor{ 1, 0, 0, 0, 0, 0, 0, 0 };
		//Motor offset{ oneMotor - 0.5f * SIZE * collisionPlane * (-e0) };
		m_pos = GAUtils::Project(m_pos, collisionPlane);
		
		MultiVector transformedVelocity{ collisionPlane * m_velocity * ~collisionPlane };
		m_velocity = Motor{ transformedVelocity[0], transformedVelocity[5], transformedVelocity[6], transformedVelocity[7],
			transformedVelocity[8], transformedVelocity[9], transformedVelocity[10], transformedVelocity[15] };
	}
}

ThreeBlade Particle::GetPos() const
{
	return m_pos;
}
