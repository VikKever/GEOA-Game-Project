#include "BoundingBox.h"
#include "GAUtils.h"

BoundingBox::BoundingBox(const Rectf& box)
	:m_leftPlane{ -box.left, 1, 0, 0 }
	, m_rightPlane{ box.left + box.width, -1, 0, 0}
	, m_bottomPlane{ -box.bottom, 0, 1, 0 }
	, m_topPlane{ box.bottom + box.height, 0, -1, 0}
{
}

bool BoundingBox::Collides(const ThreeBlade& point, OneBlade& collision, float offset) const
{
	if ((point & collision) - offset < 0.f)
	{
		collision = m_leftPlane;
		return true;
	}
	else if ((point & collision) - offset < 0.f)
	{
		collision = m_rightPlane;
		return true;
	}
	else if ((point & collision) - offset < 0.f)
	{
		collision = m_bottomPlane;
		return true;
	}
	else if ((point & collision) - offset < 0.f)
	{
		collision = m_topPlane;
		return true;
	}
	return false;
}
