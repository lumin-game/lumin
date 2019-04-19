#include "LightBeam.hpp"
#include <math.h>       /* sqrt */
#include "LightBeamParticle.hpp"

void LightBeam::setParameters(vec2 dest, Switch* parent) {
	destination = dest;
	parent_switch = parent;
}

void LightBeam::update(float ms) {
	const float MOVEMENT_STEP= 80.f;

	vec2 distanceVector = { destination.x - m_position.x, destination.y - m_position.y };
	float distance = sqrt((distanceVector.x*distanceVector.x) + (distanceVector.y * distanceVector.y));
	float xNormalized = (distanceVector.x / distance) * (MOVEMENT_STEP*(ms/100));
	float yNormalized = (distanceVector.y / distance) * (MOVEMENT_STEP*(ms / 100));

	float distWillTravel = sqrt((yNormalized*yNormalized) + (xNormalized*xNormalized));
	
	if (distWillTravel <= distance) {
		//TODO only spawn particles if x ms has passed

		m_position = { m_position.x + xNormalized, m_position.y + yNormalized };

		LightBeamParticle* particle = new LightBeamParticle();
		particle->init(m_position.x, m_position.y);
		particle->set_lit(true);
		parent_switch->add_beam_particle(particle);
	}
	else {
		//TODO destroy this beam here
	}
}