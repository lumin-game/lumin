#pragma once

#include "entity.hpp"
#include <iostream>

class SwitchableEntity : public Entity {
public:

    ~SwitchableEntity() override { Entity::destroy(); }

    virtual void on_switch(bool state);

    bool get_is_on();

protected:
    bool m_is_on;
};