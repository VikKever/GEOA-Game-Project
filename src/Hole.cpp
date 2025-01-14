#include "Hole.h"
#include "Ball.h"
#include "utils.h"
#include "Game.h"

Hole::Hole(const ThreeBlade& position)
	:m_pos{position}
{
}

void Hole::Draw() const
{
	utils::SetColor(Color4f{ 0.2f, 0.2f, 0.2f, 1 });
	utils::FillEllipse(m_pos[0], m_pos[1], SIZE / 2, SIZE / 2);
}

bool Hole::CheckFallsIn(Ball& ball) const
{
	// a ball falls in if it touches the center of the hole
	if ((ball.GetFlatPos() & m_pos).Norm() < (Ball::SIZE / 2) + (Hole::SIZE / 2))
	{
		ball.Kill();
		if (ball.IsProjectile())
		{
			Game::AddLives(-1);
		}
		else
		{
			Game::AddLives(1);
		}
		return true;
	}
	return false;
}
