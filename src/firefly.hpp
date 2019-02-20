#pragma once
#include <vector>
#include <common.hpp>
#include <light_mesh.hpp>

class Firefly : public Renderable
{
	const int FIREFLY_COUNT = 6;
	const float FIREFLY_DISTRIBUTION = 30.f;
	const float FIREFLY_RADIUS = 5.f;

	struct SingleFirefly
	{
		SingleFirefly(float x, float y)
		{
			position = { x, y };
			force = { 0.f, 0.f };
		}

		vec2 position;
		vec2 force;
	};

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Update player position
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the player
	void draw(const mat3& projection) override;

	vec2 get_position() const;
	vec2 get_screen_pos() const;
	void set_screen_pos(vec2 screenPos);

private:
	std::vector<SingleFirefly> fireflies;
	vec2 m_scale;
	vec2 m_position;
	vec2 m_screen_pos;
	LightMesh lightMesh;
};
