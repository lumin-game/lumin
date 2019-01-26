#pragma once

#include "common.hpp"


class Water : public Renderable
{
public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Renders the water
	void draw(const mat3& projection)override;

private:
};
