#include <random>
#include "lantern.hpp"
#include "CollisionManager.hpp"

bool Lantern::init(float x_pos, float y_pos) {
    Entity::init(x_pos, y_pos);
    std::random_device rand;
    std::mt19937 gen(rand());
    std::uniform_real_distribution<> dis(-FIREFLY_DISTRIBUTION, FIREFLY_DISTRIBUTION);
    for (int i = 0; i < FIREFLY_COUNT; ++i) {
        fireflies.emplace_back(SingleFirefly((float) dis(gen), (float) dis(gen)));
    }

    lightMesh.init();
    return true;
}

void Lantern::update(float ms) {

    // after the lantern is turned on, have fireflies appear one by one until all 12 are visible
    if (ms_since_activation < MAX_MS_SINCE_ACTIVATION && get_lit()) {
        ms_since_activation += ms;
        num_fireflies_drawn = std::min(FIREFLY_COUNT - 1, (int) (ms_since_activation / MS_BETWEEN_SPAWN));
    }
    for (SingleFirefly &firefly : fireflies) {
        firefly.position += firefly.velocity * ms;
        firefly.position = {std::clamp(firefly.position.x, -LANTERN_MAX_RANGE, LANTERN_MAX_RANGE),
                            std::clamp(firefly.position.y, -LANTERN_MAX_RANGE, LANTERN_MAX_RANGE)};
        firefly.velocity += firefly.CalculateForce(fireflies) * ms;
    }
}

void Lantern::draw(const mat3 &projection) {
    Entity::draw(projection);
    // same as Firefly::draw(), except don't draw all the SingleFireflies at once when lantern is turned on
    if (get_lit()) {
        SingleFirefly::ParentData fireflyData;
        // the firefly jar is in bottom half of the lantern texture, so move the fireflies down
        fireflyData.m_position = vec2{m_position.x, m_position.y + 18};

        int i = 0;
        for (SingleFirefly& firefly : fireflies)
        {
            if (i > num_fireflies_drawn) {
                break;
            }

            firefly.parent = fireflyData;
            firefly.draw(projection);
            i++;
        }

        RadiusLightMesh::ParentData lightData;
        lightData.m_position = m_position;
        lightMesh.SetParentData(lightData);
        lightMesh.draw(projection);
    }
}

void Lantern::destroy() {
    Entity::destroy();
    Firefly::destroy();
}

void Lantern::activate() {
    set_lit(true);
}
