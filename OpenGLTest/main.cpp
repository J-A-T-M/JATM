#define GLEW_STATIC
#include <GLEW/glew.h>
#include <glm/glm.hpp>
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "AssetLoader.h"
#include "Enums.h"
#include "InputManager.h"
#include "Network.h"
#include "Renderer.h"
#include "Renderable.h"

GLFWwindow* window;
std::mutex mtx;
std::condition_variable cv;

// temporary garbage related to networking
const bool isServer = true;
std::thread networkThread;
std::vector<std::shared_ptr<Renderable>> players;
std::vector<InputSourceEnum> playerInputSources = { INPUT_LOCAL1, INPUT_LOCAL2, INPUT_CLIENT1, INPUT_CLIENT2, INPUT_CLIENT3, INPUT_CLIENT4, INPUT_CLIENT5, INPUT_CLIENT6 };

void movePlayersBasedOnInput() {
	for (int i = 0; i < players.size() && i < playerInputSources.size(); i++) {
		Input input = InputManager::getInput(playerInputSources[i]);
		float xAxis = input.right - input.left;
		float zAxis = input.down - input.up;
		glm::vec3 movement = glm::vec3(xAxis, 0.0f, zAxis);
		if (movement != glm::vec3(0)) {
			movement = normalize(movement);
		}
		players[i]->position += movement * 0.5f;
	}

	for (int i = 0; i < players.size(); i++) {
		serverState.players[i].x = players[i]->position.x;
		serverState.players[i].z = players[i]->position.z;
	}
}

void movePlayersBasedOnNetworking() {
	for (int i = 0; i < players.size(); i++) {
		players[i]->position.x = serverState.players[i].x;
		players[i]->position.z = serverState.players[i].z;
	}
}

int main() {
	if (isServer) {
		networkThread = std::thread(listenForClients);
	} else {
		networkThread = std::thread(ClientLoop);
	}
	
	AssetLoader::preloadAssets();
	Renderer renderer;
	std::unique_lock<std::mutex> lck(mtx);
	std::thread renderThread = std::thread(&Renderer::RenderLoop, &renderer);
	cv.wait(lck);
	InputManager::registerInputCallbacks(window);

	for (int i = 0; i < MAX_CLIENTS + NUM_LOCAL; i++) {
		Renderable *playerRenderable = new Renderable();
		playerRenderable->position = glm::vec3(10.0 * i - 15.0, 1.0, 5.0);
		playerRenderable->scale = glm::vec3(2.0f);
		playerRenderable->color = glm::vec4(1.0, 0.25, 0.1, 1.0);
		playerRenderable->model = MODEL_SUZANNE;
		std::shared_ptr<Renderable> player_renderable_ptr(playerRenderable);
		EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(player_renderable_ptr), false);
		players.push_back(player_renderable_ptr);
	}
	
	Renderable floor;
	floor.roughness = 0.8;
	floor.color = glm::vec4(0.25, 0.25, 0.25, 1.0);
	floor.scale = glm::vec3(64);
	floor.position = glm::vec3(0, -33, 0);
	floor.model = MODEL_CUBE;
	std::shared_ptr<Renderable> ptr2(&floor);
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(ptr2), false);

	renderer.light_direction.y = -0.5;

	for (int i = 0;; i++) {
		Sleep(16);
		renderer.light_direction.z = sin(i * 0.01);
		renderer.light_direction.x = cos(i * 0.01);

		if (isServer) {
			movePlayersBasedOnInput();
			// send player positions to clients
			sendToClients();
		} else {
			movePlayersBasedOnNetworking();
			// send user input to server
			sendToServer();
		}
	}

	return 0;
}