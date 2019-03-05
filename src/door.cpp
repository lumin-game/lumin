#include "common.hpp"
#include "door.hpp"

bool Door::is_player_inside(Player* player) {
	return CollisionManager::GetInstance().BoxCollide(m_position, get_bounding_box(), player->get_position(), { 0.f, 0.f });
}
