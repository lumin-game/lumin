#include "switch.hpp"
#include "CollisionManager.hpp"

void Switch::activate() {
	Mix_PlayChannel(-1, get_sound(), 0);
	for (auto* entity : m_entities) {
		if (entity != nullptr) {
			entity->activate();
		} else {
			m_entities.erase(entity);
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
		} else {
			m_entities.erase(entity);
		}
	}
}

void Switch::set_toggle_switch(bool isToggle) {
	mToggleSwitch = isToggle;
	
	// Reinitialize the entity so that we get the proper texture (hehe)
	Entity::init(get_position().x, get_position().y);
}
