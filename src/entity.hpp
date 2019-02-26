#pragma once

#include "common.hpp"
#include "player.hpp"
#include <vector>
#include <iostream>

struct EntityColor {
	float r, g, b, a;
};

class Entity : public Renderable {
public:
	// Things the subclass need to implement
	virtual ~Entity() = default;

	virtual char* get_texture_path() const = 0;
	virtual char* get_lit_texture_path() const = 0;
	virtual bool is_player_collidable() const = 0;
	virtual bool is_light_collidable() const = 0;
	virtual bool is_light_dynamic() const = 0;
	virtual EntityColor get_color() const = 0;

	// Creates all the associated render resources and default transform
	bool init(int x_pos, int y_pos);

	// Releases all the associated resources
	void destroy();

	// Update logic for entities
	void update(Player* player);

	// Renders the entity using the texture
	void draw(const mat3& projection) override;

	// Returns the current entity position
	vec2 get_position() const;

	// Sets the new entity position
	void set_position(vec2 position);

	// Returns the current entity screen position
	vec2 get_screen_pos() const;

	// Sets the new entity screen position
	void set_screen_pos(vec2 position);

	// Returns the wall's bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;

	virtual ParametricLines calculate_static_equations() const;
	virtual ParametricLines calculate_dynamic_equations() const;

	void set_lit(bool lit);
	bool get_lit() const;

private:
    // pointer to the active texture
	Texture unlit_texture;
	Texture lit_texture;
	bool m_is_lit = false;
protected:
    // Window coordinates
    vec2 m_screen_pos;
    Texture* texture;
    // 1.f in each dimension. 1.f is as big as the associated texture
    vec2 m_scale;
    vec2 m_position;
};
