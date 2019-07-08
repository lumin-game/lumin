#pragma once

#include "entity.hpp"

class Wall : public Entity {
public:
	struct NeighborIsWall
	{
		bool top;
		bool bottom;
		bool right;
		bool left;
	};

private:
	NeighborIsWall neighbors;

public:
	const char* get_texture_path() const override { return textures_path("wall.png"); }
	const char* get_lit_texture_path() const override { return textures_path("wall.png"); }
	bool is_player_collidable() const override { return true; }
	bool is_light_collidable() const override { return true; }
	virtual bool no_neighboring_walls() const { return true; }
	virtual bool is_light_dynamic() const override { return no_neighboring_walls(); }

	ParametricLines calculate_static_equations() const override { return ParametricLines(); };
	ParametricLines calculate_dynamic_equations() const override;

	NeighborIsWall& GetNeighborStruct() { return neighbors; };
};
