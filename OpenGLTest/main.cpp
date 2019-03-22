#include <chrono>
#include <iostream>

#include "AssetLoader.h"
#include "Renderer.h"
#include "MainScene.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;


int main() {

	std::cout << "Enter 1 for server, 0 for client" << std::endl;
	char c = std::cin.get();

	bool isServer = (c == '1');

	std::string serverIP;

	if (!isServer) {
		std::cin.ignore();
		std::cout << "Enter IP: ";
		std::getline(std::cin, serverIP);
	}


	AssetLoader::preloadAssets();
	Renderer* renderer = new Renderer();

	
	Scene* scene = new MainScene(isServer, serverIP);
	scene->Setup();

	const double FIXED_UPDATE_FREQUENCY = 20.0;
	const double FIXED_DELTA_TIME = 1.0 / FIXED_UPDATE_FREQUENCY;

	auto start = high_resolution_clock::now();
	auto end = high_resolution_clock::now();
	double accumulator = 0.0;
    while (!scene->Done() && !renderer->Done()) {
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
	return EXIT_SUCCESS;
}