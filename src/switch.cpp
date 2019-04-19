#include "switch.hpp"
#include "CollisionManager.hpp"
#include "LightBeam.hpp"

void Switch::activate() {
	Mix_PlayChannel(-1, get_sound(), 0);
	for (auto* entity : m_entities) {
		if (entity != nullptr) {
			entity->activate();

			LightBeam* lightBeam = new LightBeam();
			lightBeam->init(m_position.x, m_position.y);
			lightBeam->setParameters(entity->get_position(), this);
			lightBeam->set_lit(true);
			light_beams.insert(lightBeam);
		}
	}
}

void Switch::deactivate() {
	if (!mToggleSwitch) {
		return;
	}

	for (auto* entity : m_entities) {
		if (entity != nullptr) {
			entity->deactivate();
		}
		else {
			m_entities.erase(entity);
		}
	}
}

void Switch::update(float ms) {
	for (Entity* light_beam : light_beams) {
		if (light_beam != nullptr) {
			light_beam->update(ms);
		}
		else {
			light_beams.erase(light_beam);
		}
	}

	std::cout << "num light beam particles" << light_beam_particles.size() << std::endl;

	for (LightBeamParticle* light_part : light_beam_particles) {
		if (light_part != nullptr && !light_part->is_destroyed()) {
			light_part->update(ms);
		}
		else {
			light_beam_particles.erase(light_part);
			delete light_part;
		}
	}


}

void Switch::draw(const mat3& projection) {
	Entity::draw(projection);
	for (Entity* light_beam : light_beams) {
		if (light_beam != nullptr) {
			light_beam->draw(projection);
		}
		else {
			light_beams.erase(light_beam);
		}
	}

	for (LightBeamParticle* light_part : light_beam_particles) {
		if (light_part != nullptr && !light_part->is_destroyed()) {
			light_part->draw(projection);
		}
		else {
			light_beam_particles.erase(light_part);
			delete light_part;
		}
	}
}

void Switch::add_beam_particle(LightBeamParticle* particle) {
		light_beam_particles.insert(particle);
}


void Switch::set_toggle_switch(bool isToggle) {
	mToggleSwitch = isToggle;

	// Reinitialize the entity so that we get the proper texture (hehe)
	Entity::init(get_position().x, get_position().y);
}
