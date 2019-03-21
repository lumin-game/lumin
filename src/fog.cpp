#include "common.hpp"
#include "fog.hpp"

ParametricLines Fog::calculate_dynamic_equations() const
{
	ParametricLines outLines;

	if (!is_light_collidable()) {
		return outLines;
	}

	// Create 4 lines for each each of the box and returns them
	vec2 boundingBox = get_bounding_box();
	float xHalf = boundingBox.x / 2;
	float yHalf = boundingBox.y / 2;

	float rightBound = m_position.x + xHalf;
	float leftBound = m_position.x - xHalf;
	float topBound = m_position.y + yHalf;
	float bottomBound = m_position.y - yHalf;

	ParametricLine rightEdge;
	rightEdge.x_0 = rightBound;
	rightEdge.x_t = 0.f;
	rightEdge.y_0 = bottomBound;
	rightEdge.y_t = topBound - bottomBound;

	ParametricLine leftEdge;
	leftEdge.x_0 = leftBound;
	leftEdge.x_t = 0.f;
	leftEdge.y_0 = bottomBound;
	leftEdge.y_t = topBound - bottomBound;

	ParametricLine topEdge;
	topEdge.x_0 = leftBound;
	topEdge.x_t = rightBound - leftBound;
	topEdge.y_0 = topBound;
	topEdge.y_t = 0.f;

	ParametricLine bottomEdge;
	bottomEdge.x_0 = leftBound;
	bottomEdge.x_t = rightBound - leftBound;
	bottomEdge.y_0 = bottomBound;
	bottomEdge.y_t = 0.f;

	if (!neighborFogs.top)
	{
		outLines.push_back(topEdge);
	}

	if (!neighborFogs.bottom)
	{
		outLines.push_back(bottomEdge);
	}

	if (!neighborFogs.right)
	{
		outLines.push_back(rightEdge);
	}

	if (!neighborFogs.left)
	{
		outLines.push_back(leftEdge);
	}

	return outLines;
}