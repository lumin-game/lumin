#include "switch.hpp"
#include "CollisionManager.hpp"

void Switch::activate() {
	Mix_PlayChannel(-1, get_sound(), 0);
	for (auto* entity : m_entities) {
		if (entity != nullptr) {
			entity->activate();

			LightBeam* lightBeam = new LightBeam();
			lightBeam->init(m_position.x, m_position.y);
			lightBeam->setParameters(entity->get_position());
			lightBeam->set_lit(true);
			light_beams.push_back(lightBeam);
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
	for (LightBeam* light_beam: light_beams) {
		light_beam->update(ms);
	}
}

void Switch::draw(const mat3& projection) {
	Entity::draw(projection);
	for (LightBeam* light_beam : light_beams) {
		light_beam->draw(projection);
	}
}


void Switch::set_toggle_switch(bool isToggle) {
	mToggleSwitch = isToggle;

	// Reinitialize the entity so that we get the proper texture (hehe)
	Entity::init(get_position().x, get_position().y);
}
