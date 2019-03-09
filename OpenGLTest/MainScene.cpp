#include "MainScene.h"
#include "Network.h"
#include <glm/glm.hpp>

MainScene::MainScene(bool isServer) : IS_SERVER(isServer) {}

void MainScene::movePlayersBasedOnInput(const float delta) {
	for (int i = 0; i < players.size(); i++) {
		glm::vec3 pos = players[i]->getLocalPosition();
		if (i < playerInputSources.size()) {
			Input input = InputManager::getInput(playerInputSources[i]);
			float xAxis = input.right - input.left;
			float zAxis = input.down - input.up;
			players[i]->setForce(glm::vec3(xAxis, 0.0f, zAxis));
		}
	}
}

void MainScene::setServerState()
{
	for (int i = 0; i < players.size(); i++) {
		glm::vec3 pos = players[i]->getLocalPosition();
		serverState.players[i].x = pos.x;
		serverState.players[i].z = pos.z;
	}
}

void MainScene::movePlayersBasedOnNetworking() {
	for (int i = 0; i < players.size(); i++) {
		glm::vec2 pos = glm::vec2(serverState.players[i].x, serverState.players[i].z);
		players[i]->setLocalPositionXZ(pos);
	}
}
int Q, R;
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
		player->setLocalPosition(glm::vec3(10.0 * i - 15.0, 2.0, 5.0));
		player->renderable->color = color[i % 4];
		player->renderable->metallic = metallic[i];
		player->renderable->interpolated = true;
		EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(player->renderable), false);
		players.push_back(player);
	}

	GameObject* floor = new GameObject();
	floor->setLocalScale(32.0f);
	floor->setLocalPosition(glm::vec3(0, -32, 0));
	floor->addRenderable();
	floor->renderable->roughness = 0.8;
	floor->renderable->color = glm::vec4(0.8, 0.6, 0.4, 1.0);
	floor->renderable->model = MODEL_CUBE;
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(floor->renderable), false);

	Q = floor->getScale() - 3;
	R = Q + Q;
	std::cout << "Q = " << Q << std::endl;
	std::cout << "R = " << R << std::endl;
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

	glm::vec3 down_color = glm::vec3(floor->renderable->color) * (up_color + -directionalLight.direction.y * directionalLight.color);
	EventManager::notify(RENDERER_SET_AMBIENT_DOWN, &TypeParam<glm::vec3>(down_color), false);
	
}
int h = 0, g = 0;
float timeTwo;
void MainScene::Update(const float delta) {
	time += delta;
	
	if (activeHazard == nullptr || activeHazard->grounded()) {
		activeHazard = new Hazard();
		activeHazard->fallSpeed = 5.0f;
		
		activeHazard->setLocalPosition(glm::vec3(rand() % R + (-Q), 15, rand() % R + (-Q)));
		activeHazard->clearRenderablePreviousTransforms();
		EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(activeHazard->renderable), false);
		hazards.push_back(activeHazard);
	}
	activeHazard->update(delta);

	if (hazards[h]->grounded()) {
		timeTwo += delta;

		std::cout << timeTwo << std::endl;

		if (timeTwo > 5) {
			hazards[h]->renderable->color = glm::vec4(0.0, 0.0, 0.0, 0.0);
			hazards[h]->setLocalScale(0.01);
			h++;
			timeTwo = 0;
		}		
	}

	if (IS_SERVER) {
		movePlayersBasedOnInput(delta);
		// check collisions
		PhysicsManager::Update(players, hazards, delta);
		// set server states
		setServerState();
		// send player positions to clients
		sendToClients();
	} else {
		movePlayersBasedOnNetworking();
		// send user input to server
		sendToServer();
	}

	EventManager::notify(FIXED_UPDATE_STARTED_UPDATING_RENDERABLES, NULL, false);
	for (GameObject* gameObject : players) {
		gameObject->updateRenderableTransforms();
	}
	for (GameObject* gameObject : hazards) {
		gameObject->updateRenderableTransforms();
	}
	EventManager::notify(FIXED_UPDATE_FINISHED_UPDATING_RENDERABLES, &TypeParam<float>(delta), false);
}

bool MainScene::Done() {
	return false;
}

void MainScene::Cleanup() {
	for (GameObject* gameObject : players) {
		if (gameObject != nullptr) {
			delete gameObject;
		}
	}
	for (GameObject* gameObject : hazards) {
		if (gameObject != nullptr) {
			delete gameObject;
		}
	}
	networkThreadShouldDie = true;
	networkThread.join();

	std::cout << "MainScene cleaned up" << std::endl;
}