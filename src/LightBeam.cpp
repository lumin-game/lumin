#include "LightBeam.hpp"
#include <math.h>       /* sqrt */

void LightBeam::setParameters(vec2 dest) {
	destination = dest;
}

void LightBeam::update(float ms) {
	const float MOVEMENT_STEP= 60.f;

	vec2 distanceVector = { destination.x - m_position.x, destination.y - m_position.y };
	float distance = sqrt((distanceVector.x*distanceVector.x) + (distanceVector.y * distanceVector.y));
	float xNormalized = distanceVector.x / distance;
	float yNormalized = distanceVector.y / distance;
	
	m_position = { m_position.x + xNormalized * (MOVEMENT_STEP*(ms/100)), m_position.y + yNormalized * (MOVEMENT_STEP*(ms/100)) };
}