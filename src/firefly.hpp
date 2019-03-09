#pragma once
#include <vector>
#include <common.hpp>
#include <light_mesh.hpp>
#include "entity.hpp"

class Firefly : public Entity {

protected:
    struct SingleFirefly : public Renderable
    {
        const float FIREFLY_RADIUS = 5.f;
        const float FIREFLY_MAX_RANGE = 20.f;

        vec2 position;
        vec2 velocity;

        vec2 CalculateForce(std::vector<SingleFirefly>& fireflies) const;

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

    };

    std::vector<SingleFirefly> fireflies;
    const int FIREFLY_COUNT = 12;
    LightMesh lightMesh;
	const float FIREFLY_DISTRIBUTION = 30.f;
public:
	const char* get_texture_path() const override { return nullptr; }

	// Creates all the associated render resources and default transform
	bool init(float x_pos, float y_pos) override;

	// Releases all associated resources
	void destroy() override;

	void update(float ms) override;

	void draw(const mat3& projection) override;

private:
    vec2 m_velocity;
};
