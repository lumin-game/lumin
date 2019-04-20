// internal
#include "common.hpp"
#include "world.hpp"

#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>
#include <iostream>

using namespace std;

using Clock = std::chrono::high_resolution_clock;

// Global 
World world;
const int width = 1200;
const int height = 800;
const float minDeltaTimeMs = 100.f;

// Entry point
int main(int argc, char* argv[])
{
	// Initializing world (after renderer.init().. sorry)
	if (!world.init({ (float)width, (float)height }))
	{
		// Time to read the error message
		std::cout << "Press any key to exit" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}


	auto t = Clock::now();

	// variable timestep loop.. can be improved (:
	while (!world.is_over())
	{
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_sec = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		elapsed_sec = fmin(elapsed_sec, minDeltaTimeMs);
		world.update(elapsed_sec);
		world.draw();
	}

	world.destroy();

	return EXIT_SUCCESS;
}