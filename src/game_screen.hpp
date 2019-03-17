#pragma once

#include "common.hpp"

class GameScreen : public Renderable
{
public:
	virtual ~GameScreen() { GameScreen::destroy(); };

	virtual const char* get_texture_path() const = 0;
	// Creates all the associated render reso+urces and default transform
	virtual bool init();

	// Releases all the associated resources
	virtual void destroy();

	void draw(const mat3& projection)override;

	void set_position(vec2 position) {
		m_position = position;
	}

	vec2 get_bounding_box()const;

protected:
	Texture screen_texture;
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	vec2 m_position;
};