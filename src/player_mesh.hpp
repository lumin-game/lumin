#pragma once

#include "common.hpp"

class PlayerMesh : public Renderable
{
public:
	struct ParentData
	{
		vec2 m_position;
	};

private:
	static Texture player_spritesheet;
	static const int TOTAL_FRAMES = 18;
	static const int FRAME_SPEED = 5;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	void update(bool is_walking);

	// Renders the player
	void draw(const mat3& projection) override;

	// Render player as facing right
	void turn_right();

	// Render player as facing left
	void turn_left();

	void SetParentData(ParentData data) { m_parent = data; }

	int GetPlayerWidth() const;

	int GetPlayerHeight() const;

private:
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	ParentData m_parent;
	int m_current_frame;
	// keeps track of how many frames has passed since the last sprite change
	int m_frame_counter;

};
