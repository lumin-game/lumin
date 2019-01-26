#pragma once

#include "common.hpp"

// Salmon enemy 
class Wall : public Renderable
{
	// Shared between all walls, no need to load one for each instance
	//TODO: change from turtle texture to actual wall texture
	static Texture wall_texture;

public:
	// Creates all the associated render resources and default transform
	bool init(int x_pos, int y_pos);

	// Releases all the associated resources
	void destroy();

	// Renders the wall
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection)override;

	// Returns the current wall position
	vec2 get_position()const;

	// Sets the new wall position
	void set_position(vec2 position);

	// Returns the wall's bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box()const;

private:
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	int m_path_type;
};