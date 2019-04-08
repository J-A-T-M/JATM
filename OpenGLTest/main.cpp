#include <chrono>

#include "AssetLoader.h"
#include "Renderer.h"
#include "StateMachine.h"
#include "MenuScene.h"
#include "UI/UIManager.h"

#include "SoundSystemClass.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;

int main() {
	AssetLoader::preloadAssets();
	Renderer* renderer = new Renderer();
	UIManager* uiManager = new UIManager(1600, 900);
	StateMachine* stateMachine = new StateMachine(new MenuScene());

	SoundSystemClass* soundSystem = new SoundSystemClass();
	
	const double FIXED_UPDATE_FREQUENCY = 20.0;
	const double FIXED_DELTA_TIME = 1.0 / FIXED_UPDATE_FREQUENCY;

	auto start = high_resolution_clock::now();
	auto end = high_resolution_clock::now();
	double accumulator = 0.0;
    while (!stateMachine->Done() && !renderer->Done()) {
		start = high_resolution_clock::now();
		accumulator += duration_cast<duration<double>>(start - end).count();
        end = start;
        while (accumulator >= FIXED_DELTA_TIME) {
            accumulator -= FIXED_DELTA_TIME;
			stateMachine->Update(FIXED_DELTA_TIME);
        }
    }

	delete stateMachine;
	delete uiManager;
	delete renderer;
	return EXIT_SUCCESS;
}