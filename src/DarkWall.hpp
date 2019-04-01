#pragma once

#include "Wall.hpp"

class DarkWall : public Wall {
public:
	const char* get_texture_path() const override { return textures_path("wall.png"); }
	const char* get_lit_texture_path() const override { return textures_path("fog.png"); }
	bool is_player_collidable() const override { return isCollidable; }
	bool is_light_collidable() const override { return isCollidable; }
	virtual EntityColor get_color() const { return EntityColor({ 0.8, 0.8, 0.8, 1.0 }); }

	virtual void activate() override;
	virtual void deactivate() override;

	virtual void update(float ms) override;

private:
	bool isCollidable = false;
	bool shouldBeCollidable = false;
};