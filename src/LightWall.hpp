#pragma once

#include "wall.hpp"

class LightWall : public Wall {
public:
	const char* get_texture_path() const override { return textures_path("invisible_wall.png"); }
	const char* get_lit_texture_path() const override { return textures_path("visible_wall.png"); }
	bool is_player_collidable() const override { return isCollidable; }
	bool is_light_collidable() const override { return isCollidable; }
	virtual EntityColor get_color() const override { return EntityColor({ 0.8, 0.8, 0.8, 1.0 }); }

	virtual void activate() override;
	virtual void deactivate() override;

	virtual void update(float ms) override;

private:
	bool isCollidable = false;
	bool shouldBeCollidable = false;
};
