#include "switch.hpp"
#include "CollisionManager.hpp"



void Switch::register_movable_wall(MovableWall* wall) {
	m_movableWalls.emplace_back(wall);
}
void Switch::register_door(Door* door) {
	m_door = door;
}

void Switch::update() {
	bool hitByLight = CollisionManager::GetInstance().IsHitByLight(this);
	if (hitByLight && !switchedOn) {
		trigger_switch_on();
		switchedOn = true;
	}
	else if (!hitByLight && switchedOn) {
		trigger_switch_off();
		switchedOn = false;
	}
}

void Switch::trigger_switch_on() {
	for (MovableWall* mov_wall : m_movableWalls) {
		mov_wall->trigger_movement(true);
	}

	if (m_door != nullptr) {
		m_door->set_lit(true);
	}
}

void Switch::trigger_switch_off() {
	// eventually we might want to have this move blocks back to their starting positions, but for now, since we don't have the kind of switch that stays
	// on permanently implemented, this switch should leave movable blocks in their location even after the switch is off
}