#include "switch.hpp"
#include "CollisionManager.hpp"

void Switch::activate() {
	Mix_PlayChannel(-1, get_sound(), 0);
	for (auto* entity : m_entities) {
		if (entity != nullptr) {
			// If a switch is connected to another switch, we treat it as
			// a "reset switch", so deactivate that switch instead.
			if (auto *s = dynamic_cast<Switch *>(entity)) {
				s->reset();
			} else {
				entity->activate();
			}
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
	}
}

void Switch::reset() {
	set_lit(false);

	for (auto* entity : m_entities) {
		if (entity != nullptr) {
			entity->deactivate();
		}
	}
}

void Switch::set_toggle_switch(bool isToggle) {
	mToggleSwitch = isToggle;

	// Reinitialize the entity so that we get the proper texture (hehe)
	Entity::init(get_position().x, get_position().y);
}
