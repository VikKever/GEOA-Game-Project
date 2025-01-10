#include "Hole.h"
#include "Ball.h"
#include "utils.h"

Hole::Hole(const ThreeBlade& position)
	:m_pos{position}
{
}

void Hole::Draw() const
{
	utils::SetColor(Color4f{ 0, 0.1f, 0, 1 });
	utils::FillEllipse(m_pos[0], m_pos[1], SIZE / 2, SIZE / 2);
}

bool Hole::FallsIn(const Ball& ball) const
{
	// a ball falls in if it touches the center of the hole
	return (ball.GetFlatPos() & m_pos).Norm() < Ball::SIZE / 2;
}
