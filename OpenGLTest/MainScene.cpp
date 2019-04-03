#include "MainScene.h"
#include <glm/glm.hpp>
#include "MenuScene.h"


MainScene::MainScene(bool isServer, std::string serverIP) : IS_SERVER(isServer), SERVER_IP(serverIP){
	EventManager::subscribe(SPAWN_HAZARD, this);

	networkManager = new NetworkManager(IS_SERVER, SERVER_IP);

	glm::vec3 color[] = { Colour::FUCSHIA , Colour::ORANGE, Colour::BLUERA , Colour::GREENRA };
	for (int i = 0; i < MAX_CLIENTS + NUM_LOCAL; i++) {
		Player* player = new Player(glm::vec2(10.0 * i - 15.0, 5.0), color[i % 4]);
		EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(player->renderable), false);
		players.push_back(player);
	}

	floor = new GameObject();
	floor->setLocalScale(32.0f);
	floor->setLocalPosition(glm::vec3(0, -32, 0));
	floor->addRenderable();
	floor->renderable->roughness = 0.8;
	floor->renderable->color = Colour::BEIGE;
	floor->renderable->model = MODEL_CUBE;
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(floor->renderable), false);

	camera.position = glm::vec3(0.0f, 64.0f, 100.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.FOV = 25.0f;
	camera.nearClip = 0.1f;
	camera.farClip = 1000.0f;
	EventManager::notify(RENDERER_SET_CAMERA, &TypeParam<Camera>(camera), false);

	directionalLight.direction = glm::normalize(glm::vec3(1.0f, -0.5f, -0.25f));
	directionalLight.color = Colour::BEIGARA;
	directionalLight.nearclip = -50.0f;
	directionalLight.farclip = 50.0f;
	EventManager::notify(RENDERER_SET_DIRECTIONAL_LIGHT, &TypeParam<DirectionalLight>(directionalLight), false);

	glm::vec3 up_color = Colour::BROWN;
	EventManager::notify(RENDERER_SET_AMBIENT_UP, &TypeParam<glm::vec3>(up_color), false);

	EventManager::notify(RENDERER_SET_FLOOR_COLOR, &TypeParam<glm::vec3>(floor->renderable->color), false);
}

MainScene::~MainScene() {
	delete networkManager;
	for (GameObject* gameObject : players) {
		if (gameObject != nullptr) {
			delete gameObject;
		}
	}
	EventManager::unsubscribe(SPAWN_HAZARD, this);
	for (GameObject* gameObject : hazards) {
		if (gameObject != nullptr) {
			delete gameObject;
		}
	}
	delete floor;
	std::cout << "MainScene cleaned up" << std::endl;
}

bool MainScene::checkDone() {
	if (!IS_SERVER && time > networkManager->MAX_DISCONNECT_TIME_MS / 1000.0f) {
		if (!networkManager->isConnectedToServer) {
			return true;
		}
	}

	int count = 0;
	for (Player* player : players) {
		if (player->getHealth() != 0) {
			++count;
		}
	}

	return (count <= 1);
}

void MainScene::movePlayersBasedOnInput(const float delta) {
	for (int i = 0; i < players.size(); i++) {
		if (i < playerInputSources.size()) {
			Input input = InputManager::getInput(playerInputSources[i]);
			float xAxis = input.right - input.left;
			float zAxis = input.down - input.up;
			players[i]->setForce(glm::vec3(xAxis, 0.0f, zAxis));
		}
	}
}

void MainScene::sendPlayerTransforms() {
	ServerPacket packet;
	packet.type = PACKET_PLAYER_TRANSFORM;
	for (int i = 0; i < players.size(); i++) {
		packet.playerTransformPacket.playerTransforms[i].position = players[i]->getLocalPosition();
		packet.playerTransformPacket.playerTransforms[i].rotation = players[i]->getLocalRotation();
		packet.playerTransformPacket.playerTransforms[i].health = players[i]->getHealth();
		packet.playerTransformPacket.playerTransforms[i].stunFrames = players[i]->getStunFrames();
	}
	
	networkManager->SendToClients(packet);

}

void MainScene::movePlayersBasedOnNetworking() {
	for (int i = 0; i < players.size(); i++) {
		players[i]->setLocalPosition(networkManager->serverState.playerTransforms[i].position);
		players[i]->setLocalRotation(networkManager->serverState.playerTransforms[i].rotation);
		players[i]->setStunFrames(networkManager->serverState.playerTransforms[i].stunFrames);
		// horrible hackjob by markus
		// find a better way to send this
		int damage = players[i]->getHealth() - networkManager->serverState.playerTransforms[i].health;
		if (damage != 0) {
			players[i]->damageHealth(damage);
		}
	}
}

void MainScene::SpawnHazard() {
	glm::vec3 pos = glm::vec3(rand() % 58 - 29, 10 + rand() % 10, rand() % 58 - 29);
	float fallSpeed = 5.0f;

	Hazard* hazard = new Hazard(pos, fallSpeed);
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(hazard->renderable), false);
	hazards.push_back(hazard);

	ServerPacket packet;
	packet.type = PACKET_HAZARD_SPAWN;
	packet.hazardSpawnPacket.spawnPosition = pos;
	packet.hazardSpawnPacket.fallSpeed = fallSpeed;
	networkManager->SendToClients(packet);
}

void MainScene::Update(const float delta) {
	time += delta;

	auto it = hazards.begin();
	while (it != hazards.end()) {
		if ((*it)->grounded()) {
			delete (*it);
			it = hazards.erase(it);
		} else {
			++it;
		}
	}

	if (IS_SERVER) {
		if (hazards.size() <= 5) {
			SpawnHazard();
		}
		movePlayersBasedOnInput(delta);
		// check collisions
		PhysicsManager::Update(players, hazards, delta);
		// set server states
		sendPlayerTransforms();
	} else {
		movePlayersBasedOnNetworking();
		// send user input to server
		networkManager->SendToServer();
	}

	_done = checkDone();

	EventManager::notify(FIXED_UPDATE_STARTED_UPDATING_RENDERABLES, NULL, false);
	for (Player* player : players) {
		player->update();
		player->updateRenderableTransforms();
	}
	for (Hazard* hazard : hazards) {
		hazard->update(delta);
		hazard->updateRenderableTransforms();
	}
	EventManager::notify(FIXED_UPDATE_FINISHED_UPDATING_RENDERABLES, &TypeParam<float>(delta), false);
}

Scene * MainScene::GetNext() {
	return new MenuScene();
}

void MainScene::notify(EventName eventName, Param* params) {
	switch (eventName) {
		case SPAWN_HAZARD: {
			TypeParam<Hazard*> *p = dynamic_cast<TypeParam<Hazard*> *>(params);
			Hazard* hazard = p->Param;
			EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(hazard->renderable), false);
			hazards.push_back(hazard);
			break;
		}

		default: {
			break;
		}
	}
}