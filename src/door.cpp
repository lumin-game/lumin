#include "common.hpp"
#include "door.hpp"

bool Door::init(float x_pos, float y_pos) {
	Entity::init(x_pos, y_pos);
	m_position.y -= 20;
	return true;
}

bool Door::is_player_inside(Player* player) {
	return CollisionManager::GetInstance().BoxCollide(m_position, get_bounding_box(), player->get_position(), { 0.f, 0.f });
}

int Door::get_level_index() {
	return m_level_index;
}

void Door::set_level_index(int level) {
	m_level_index = level;
}
