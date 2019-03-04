#include "common.hpp"
#include "door.hpp"

bool Door::get_player_in(vec2 player_pos) {
    float door_width = texture->width * m_scale.x;
    float door_height = texture->height * m_scale.y;

	m_is_player_in = CollisionManager::GetInstance().BoxCollide(m_position, get_bounding_box(), player_pos, { 0.f, 0.f });
    return m_is_player_in;
}