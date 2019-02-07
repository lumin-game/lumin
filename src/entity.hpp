#pragma once

#include "common.hpp"
#include <vector>

class Entity : public Renderable {
	static Texture texture;

public:
	// Things the subclass need to implement
	virtual ~Entity() = default;

	virtual const char* get_texture_path() = 0;
	virtual bool is_player_collidable() const = 0;
	virtual bool is_light_collidable() const = 0;

	// Creates all the associated render resources and default transform
	bool init(int x_pos, int y_pos);

	// Releases all the associated resources
	void destroy();

	// Renders the entity using the texture
	void draw(const mat3& projection);

	// Returns the current wall position
	vec2 get_position() const;

	// Sets the new wall position
	void set_position(vec2 position);

	// Returns the wall's bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;

	ParametricLines calculate_static_equations() const;

private:
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	int m_path_type;
};
