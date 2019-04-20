#include "LightBeam.hpp"
#include <math.h>       /* sqrt */
#include "LightBeamParticle.hpp"

bool LightBeam::init(float pos_x, float pos_y) {
	m_position = { pos_x, pos_y };
	return true;
}

void LightBeam::setParameters(vec2 dest, Switch* parent) {
	destination = dest;
	parent_switch = parent;
}

void LightBeam::update(float ms) {
	const float MOVEMENT_STEP= 120.f;

	vec2 distanceVector = { destination.x - m_position.x, destination.y - m_position.y };
	float distance = sqrt((distanceVector.x*distanceVector.x) + (distanceVector.y * distanceVector.y));
	float xNormalized = (distanceVector.x / distance) * (MOVEMENT_STEP*(ms/100));
	float yNormalized = (distanceVector.y / distance) * (MOVEMENT_STEP*(ms / 100));

	float distWillTravel = sqrt((yNormalized*yNormalized) + (xNormalized*xNormalized));
	
	if (distWillTravel <= distance) {
		m_position = { m_position.x + xNormalized, m_position.y + yNormalized };

		ms_since_last_particle += ms;
		if (ms_since_last_particle > MS_BETWEEN_PARTICLES) {
			LightBeamParticle* particle = new LightBeamParticle();
			particle->init(m_position.x, m_position.y);
			parent_switch->add_beam_particle(particle);
			ms_since_last_particle -= MS_BETWEEN_PARTICLES;
		}

	}
	else {
		destroyed = true;
		destroy();
	}
}


void LightBeam::draw(const mat3& projection) {
	//do not
}

bool LightBeam::is_destroyed() {
	return destroyed;
}