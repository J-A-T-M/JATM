#include "MainScene.h"
#include "Network.h"
#include <glm/glm.hpp>

MainScene::MainScene(bool isServer) : IS_SERVER(isServer) {}

void MainScene::movePlayersBasedOnInput(const float delta) {
	const float PLAYER_SPEED = 30.0;

	for (int i = 0; i < players.size() && i < playerInputSources.size(); i++) {
		Input input = InputManager::getInput(playerInputSources[i]);
		float xAxis = input.right - input.left;
		float zAxis = input.down - input.up;
		glm::vec3 movement = glm::vec3(xAxis, 0.0f, zAxis);
		if (movement != glm::vec3(0)) {
			movement = normalize(movement);
		}
		players[i]->position += movement * PLAYER_SPEED * delta;
	}

	for (int i = 0; i < players.size(); i++) {
		serverState.players[i].x = players[i]->position.x;
		serverState.players[i].z = players[i]->position.z;
	}
}

void MainScene::movePlayersBasedOnNetworking() {
	for (int i = 0; i < players.size(); i++) {
		players[i]->position.x = serverState.players[i].x;
		players[i]->position.z = serverState.players[i].z;
	}
}

void MainScene::Setup() {
	if (IS_SERVER) {
		networkThread = std::thread(listenForClients);
	} else {
		networkThread = std::thread(ClientLoop);
	}

	for (int i = 0; i < MAX_CLIENTS + NUM_LOCAL; i++) {
		std::shared_ptr<Renderable> player(new Renderable());
		player->position = glm::vec3(10.0 * i - 15.0, 1.0, 5.0);
		player->scale = glm::vec3(2.0f);
		player->color = glm::vec4(1.0, 0.25, 0.1, 1.0);
		player->model = MODEL_SUZANNE;
		EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(player), false);
		players.push_back(player);
	}

	floor = std::make_shared<Renderable>();
	floor->roughness = 0.8;
	floor->color = glm::vec4(0.25, 0.25, 0.25, 1.0);
	floor->scale = glm::vec3(64);
	floor->position = glm::vec3(0, -33, 0);
	floor->model = MODEL_CUBE;
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(floor), false);

	camera.position = glm::vec3(0.0f, 64.0f, 100.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.FOV = 25.0f;
	camera.nearClip = 0.1f;
	camera.farClip = 1000.0f;
	EventManager::notify(RENDERER_SET_CAMERA, &TypeParam<Camera>(camera), false);

	directionalLight.direction = glm::vec3(0.0f, -0.5f, 1.0f);
	directionalLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
	directionalLight.nearclip = -50.0f;
	directionalLight.farclip = 50.0f;
	EventManager::notify(RENDERER_SET_DIRECTIONAL_LIGHT, &TypeParam<DirectionalLight>(directionalLight), false);
}

void MainScene::Update(const float delta) {
	time += delta;

	if (IS_SERVER) {
		movePlayersBasedOnInput(delta);
		// send player positions to clients
		sendToClients();
	} else {
		movePlayersBasedOnNetworking();
		// send user input to server
		sendToServer();
	}

	directionalLight.direction.x = sin(time * 0.6);
	directionalLight.direction.z = cos(time * 0.6);
	EventManager::notify(RENDERER_SET_DIRECTIONAL_LIGHT, &TypeParam<DirectionalLight>(directionalLight), false);
}

bool MainScene::Done() {
	return false;
}

void MainScene::Cleanup() {
	for (auto &player : players) {
		player.reset();
	}
	floor.reset();

	networkThreadShouldDie = true;
	networkThread.join();

	std::cout << "MainScene cleaned up" << std::endl;
}