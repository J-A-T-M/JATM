#include "MainScene.h"
#include "Network.h"
#include <glm/glm.hpp>

MainScene::MainScene(bool isServer) : IS_SERVER(isServer) {}

void MainScene::movePlayersBasedOnInput(const float delta) {
	const float PLAYER_SPEED = 30.0;

	for (int i = 0; i < players.size(); i++) {
		glm::vec3 pos = players[i]->getLocalPosition();
		if (i < playerInputSources.size()) {
			Input input = InputManager::getInput(playerInputSources[i]);
			float xAxis = input.right - input.left;
			float zAxis = input.down - input.up;
			glm::vec3 movement = glm::vec3(xAxis, 0.0f, zAxis);
			if (movement != glm::vec3(0)) {
				movement = normalize(movement);
			}
			pos += movement * PLAYER_SPEED * delta;
			players[i]->setLocalPosition(pos);
		}
		serverState.players[i].x = pos.x;
		serverState.players[i].z = pos.z;
	}
}

void MainScene::movePlayersBasedOnNetworking() {
	for (int i = 0; i < players.size(); i++) {
		glm::vec2 pos = glm::vec2(serverState.players[i].x, serverState.players[i].z);
		players[i]->setLocalPosition(pos);
	}
}

void MainScene::Setup() {
	if (IS_SERVER) {
		networkThread = std::thread(listenForClients);
	} else {
		networkThread = std::thread(ClientLoop);
	}

	glm::vec4 color[] = { glm::vec4(1.0, 0.0, 0.3, 1.0), glm::vec4(1.0, 0.3, 0.0, 1.0), glm::vec4(1.0, 0.0, 0.3, 1.0) , glm::vec4(1.0, 0.3, 0.0, 1.0) };
	float metallic[] = { 1.0f, 1.0f, 0.0f, 0.0f };
	for (int i = 0; i < MAX_CLIENTS + NUM_LOCAL; i++) {
		Player* player = new Player();
		player->setLocalPosition(glm::vec3(10.0 * i - 15.0, 1.0, 5.0));
		player->renderable->color = color[i % 4];
		player->renderable->metallic = metallic[i];
		EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(player->renderable), false);
		players.push_back(player);
	}

	floor = std::make_shared<Renderable>();
	floor->roughness = 0.8;
	floor->color = glm::vec4(0.8, 0.6, 0.4, 1.0);
	floor->scale = 64.0f;
	floor->position = glm::vec3(0, -33, 0);
	floor->model = MODEL_CUBE;
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(floor), false);

	camera.position = glm::vec3(0.0f, 64.0f, 100.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.FOV = 25.0f;
	camera.nearClip = 0.1f;
	camera.farClip = 1000.0f;
	EventManager::notify(RENDERER_SET_CAMERA, &TypeParam<Camera>(camera), false);

	directionalLight.direction = glm::normalize(glm::vec3(1.0f, -0.5f, -0.25f));
	directionalLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
	directionalLight.nearclip = -50.0f;
	directionalLight.farclip = 50.0f;
	EventManager::notify(RENDERER_SET_DIRECTIONAL_LIGHT, &TypeParam<DirectionalLight>(directionalLight), false);

	glm::vec3 up_color = glm::vec3(0.25f, 0.15f, 0.1f);
	EventManager::notify(RENDERER_SET_AMBIENT_UP, &TypeParam<glm::vec3>(up_color), false);

	glm::vec3 down_color = glm::vec3(floor->color) * (up_color + -directionalLight.direction.y);
	EventManager::notify(RENDERER_SET_AMBIENT_DOWN, &TypeParam<glm::vec3>(down_color), false);
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

	for (int i = 0; i < players.size(); i++) {
		players[i]->updateRenderableTransforms();
	}
	EventManager::notify(FIXED_UPDATE_FINISHED, &TypeParam<float>(delta), false);
}

bool MainScene::Done() {
	return false;
}

void MainScene::Cleanup() {
	for (auto &player : players) {
		delete player;
	}
	floor.reset();

	networkThreadShouldDie = true;
	networkThread.join();

	std::cout << "MainScene cleaned up" << std::endl;
}