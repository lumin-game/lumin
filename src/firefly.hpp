#pragma once
#include <vector>
#include <common.hpp>
#include <light_mesh.hpp>

class Firefly
{
private:
	const int FIREFLY_COUNT = 12;
	const float FIREFLY_DISTRIBUTION = 30.f;

	struct SingleFirefly : public Renderable
	{
		const float FIREFLY_RADIUS = 5.f;
		const float FIREFLY_MAX_RANGE = 20.f;

	public:
		struct ParentData
		{
			vec2 m_position;
			vec2 m_screen_pos;
		};

		ParentData parent;

		SingleFirefly(float x, float y)
		{
			position = { x, y };
			velocity = { 0.f, 0.f };
			init();
		}

		void destroy();
		void update(float ms, std::vector<SingleFirefly>& fireflies);
		void draw(const mat3& projection) override;

	private:
		bool init();

		vec2 position;
		vec2 velocity;

		vec2 CalculateForce(std::vector<SingleFirefly>& fireflies) const;
	};

public:
	~Firefly() { destroy(); }

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Update player position
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	void draw(const mat3& projection);

	vec2 get_position() const;
	void set_position(vec2 position);
	void set_screen_pos(vec2 screenPos);

private:
	std::vector<SingleFirefly> fireflies;
	vec2 m_scale;
	vec2 m_position;
	vec2 m_screen_pos;
	LightMesh lightMesh;
};
