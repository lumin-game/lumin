#include "switch.hpp"
#include "CollisionManager.hpp"


void Switch::update() {
	bool hitByLight = CollisionManager::GetInstance().IsHitByLight(this);
	if (hitByLight && !switchedOn) {
		activate();
		switchedOn = true;
	}
	else if (!hitByLight && switchedOn) {
		deactivate();
		switchedOn = false;
	}
}

void Switch::activate() {
	for (auto* entity : m_entities) {
		entity->activate();
	}
}

void Switch::deactivate() {
	// eventually we might want to have this move blocks back to their starting positions,
	// but for now, since we don't have the kind of switch that stays on permanently implemented,
	// this switch should leave movable blocks in their location even after the switch is off
}
