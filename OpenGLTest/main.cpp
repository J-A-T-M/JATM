#include <chrono>
#define GLEW_STATIC
#include <GLEW/glew.h>
#include <glm/glm.hpp>
#include <iostream>

#include "AssetLoader.h"
#include "InputManager.h"

#include "Renderer.h"
#include "MainScene.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;

int main() {
	AssetLoader::preloadAssets();
	Renderer* renderer = new Renderer();
	Scene* scene = new MainScene();
	scene->Setup();

	const double FIXED_UPDATE_FREQUENCY = 60.0;
	const double FIXED_DELTA_TIME = 1.0 / FIXED_UPDATE_FREQUENCY;
	auto start = high_resolution_clock::now();
	auto end = high_resolution_clock::now();
	double accumulator = 0.0;

    while (!scene->Done()) {
		start = high_resolution_clock::now();
		accumulator += duration_cast<duration<double>>(start - end).count();
        end = start;
        while (accumulator >= FIXED_DELTA_TIME) {
            accumulator -= FIXED_DELTA_TIME;
			scene->Update(FIXED_DELTA_TIME);
        }
    }
	scene->Cleanup();

	delete scene;
	delete renderer;
	return 0;
}