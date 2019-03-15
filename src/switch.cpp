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
	// eventually we might want to have this move blocks back to their starting positions,
	// but for now, since we don't have the kind of switch that stays on permanently implemented,
	// this switch should leave movable blocks in their location even after the switch is off
}
