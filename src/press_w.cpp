#include "press_w.hpp"

float PressW::update()
{
  float offset = 10 * std::sin(100000 * time(NULL)) - 110;
	return offset;
}
