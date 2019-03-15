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
	static Texture player_texture;
	static Texture player_spritesheet;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	void update(float ms);

	// Renders the player
	void draw(const mat3& projection) override;

	// Render player as facing right
	void turn_right();

	// Render player as facing left
	void turn_left();

	void playAnimation();

	void SetParentData(ParentData data) { m_parent = data; }

	int GetPlayerWidth() const;

	int GetPlayerHeight() const;

private:
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	ParentData m_parent;
	int m_total_frames;
	int m_current_frame;
	int m_frame_counter;
	int m_frame_speed;

};
