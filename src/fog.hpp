#pragma once

#include "entity.hpp"

class Fog : public Entity {

public:
	struct NeighborIsFog
	{
		bool top;
		bool bottom;
		bool right;
		bool left;
	};

private:
	NeighborIsFog neighborFogs;

public:
    const char* get_texture_path() const override { return textures_path("fog.png"); }
    bool is_light_collidable() const override { return true; }
	bool is_light_dynamic() const { return true; }

	ParametricLines calculate_static_equations() const override { return ParametricLines(); };
	ParametricLines calculate_dynamic_equations() const override;

	NeighborIsFog& GetNeighborFogStruct() { return neighborFogs; };
};
