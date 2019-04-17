#pragma once

#include "entity.hpp"
#include "CollisionManager.hpp"

class Hint : public Entity {
public:
    const char* get_texture_path() const override { return m_hint_path.c_str(); }

    void set_hint_path(const std::string &hint_path);

protected:
    std::string m_hint_path = PROJECT_SOURCE_DIR "./data/hints/loading.png";
};
