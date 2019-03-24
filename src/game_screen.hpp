#pragma once

#include "common.hpp"

class GameScreen : public Renderable
{
public:
	virtual ~GameScreen() { GameScreen::destroy(); };

	virtual const char* get_texture_path() const = 0;
	// Creates all the associated render resources and default transform
	virtual bool init(vec2 screen);

	// Releases all the associated resources
	virtual void destroy();

	void draw(const mat3& projection)override;

	virtual void set_position(vec2 position, vec2 offset = { 0, 0 }) {
		m_position = position + offset;
	}

	virtual const vec2 set_translation_scale() { return { 1.f, 1.f }; };
	virtual const vec2 set_translation_position() { return { 0, 0 }; };

	vec2 get_bounding_box()const;

protected:
	Texture screen_texture;
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	vec2 m_position;
};
