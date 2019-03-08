#pragma once
#include <vector>
#include <common.hpp>
#include <radiuslight_mesh.hpp>
#include "entity.hpp"

class Firefly : public Entity {
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

		SingleFirefly(float x, float y) {
			init(x, y);
		}

		void destroy();
		void update(float ms, std::vector<SingleFirefly>& fireflies);
		void draw(const mat3& projection) override;

	private:
		bool init(float x_pos, float y_pos);

		vec2 position;
		vec2 velocity;

		vec2 CalculateForce(std::vector<SingleFirefly>& fireflies) const;
	};

public:
	const char* get_texture_path() const override { return nullptr; }

	// Creates all the associated render resources and default transform
	bool init(float x_pos, float y_pos) override;

	// Releases all associated resources
	void destroy() override;

	void update(float ms) override;

	void draw(const mat3& projection) override;

private:
	std::vector<SingleFirefly> fireflies;
	vec2 m_velocity;
	RadiusLightMesh lightMesh;
};
