#include "MainScene.h"

#include <glm/glm.hpp>
#include "MenuScene.h"
#include "HazardFactory.h"
#include "PhysicsManager.h"
#include "EventManager.h"

MainScene::MainScene(bool isServer, std::string serverIP, int numLocal, int numRemote) : 
	IS_SERVER(isServer), 
	SERVER_IP(serverIP), 
	NUM_LOCAL(isServer ? max(numLocal, 1): 2), 
	NUM_REMOTE(isServer ? numRemote : 2) {

	networkManager = new NetworkManager(IS_SERVER, SERVER_IP);
	// horrible hackjob so we can send existing hazards to new clients
	networkManager->hazards = &hazards;

	glm::vec3 color[] = { Colour::FUCSHIA , Colour::ORANGE, Colour::BLUERA , Colour::GREENRA };
	for (int i = 0; i < NUM_LOCAL; ++i) {
		playerInputSources.push_back(InputSourceEnum(INPUT_LOCAL1 + i));
		std::shared_ptr<Player> player = std::make_shared<Player>(glm::vec2(-10.0 * i - 5, 5.0), color[i % 4]);
		players.push_back(player);
	}
	for (int i = 0; i < NUM_REMOTE; ++i) {
		playerInputSources.push_back(InputSourceEnum(INPUT_CLIENT1 + i));
		std::shared_ptr<Player> player = std::make_shared<Player>(glm::vec2(10.0 * i + 5, 5.0), color[(i + 2) % 4]);
		players.push_back(player);
	}

	if (!IS_SERVER) {
		for (int i = 0; i < players.size(); ++i) {
			players[i]->setLocalPositionY(-2);
			players[i]->clearRenderablePreviousTransforms();
		}
	}
	
	floor = std::make_shared<GameObject>();
	floor->setSize(32.0f);
	floor->setLocalPosition(glm::vec3(0, -32, 0));
	floor->addRenderable(Colour::BEIGE, MODEL_CUBE, TEXTURE_NONE, 0.8f, 0.0f);

	Camera camera;
	camera.position = glm::vec3(0.0f, 64.0f, 100.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.FOV = 25.0f;
	camera.nearClip = 0.1f;
	camera.farClip = 1000.0f;

	DirectionalLight directionalLight;
	directionalLight.direction = glm::normalize(glm::vec3(1.0f, -0.5f, -0.25f));
	directionalLight.color = Colour::BEIGARA;
	directionalLight.nearclip = -50.0f;
	directionalLight.farclip = 50.0f;

	EventManager::notify(RENDERER_SET_CAMERA, &TypeParam<Camera>(camera));
	EventManager::notify(RENDERER_SET_DIRECTIONAL_LIGHT, &TypeParam<DirectionalLight>(directionalLight));
	EventManager::notify(RENDERER_SET_AMBIENT_UP, &TypeParam<glm::vec3>(Colour::BROWN));
	EventManager::notify(RENDERER_SET_FLOOR_COLOR, &TypeParam<glm::vec3>(floor->renderable->color));
	EventManager::notify(PLAY_BGM_N, &TypeParam<int>(1));
	EventManager::notify(PLAY_SE, &TypeParam<int>(4));
	
	EventManager::subscribe(SPAWN_HAZARD, this);
}

MainScene::~MainScene() {
	delete networkManager;
	EventManager::unsubscribe(SPAWN_HAZARD, this);
	std::cout << "MainScene cleaned up" << std::endl;
}

bool MainScene::checkDone() {
	if (!IS_SERVER && time > networkManager->MAX_DISCONNECT_TIME_MS / 1000.0f) {
		if (!networkManager->isConnectedToServer) {
			return true;
		}
	}

	int count = 0;
	for (auto &player : players) {
		if (player->getHealth() != 0) {
			++count;
		}
	}
	if (count <= 1) {
		EventManager::notify(PLAY_SE, &TypeParam<int>(3));
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
	// horrible hackjob by markus
	// find a better way to send this
	// hide other players underground
	for (int i = players.size(); i < 4; i++) {
		packet.playerTransformPacket.playerTransforms[i].position = glm::vec3(0.0f, -10.0f, 0.0f);
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
		// horrible hackjob by markus
		// find a better way to send this
		int stun = networkManager->serverState.playerTransforms[i].stunFrames;
		if (stun == Player::MAX_STUN_FRAMES) {
			players[i]->setStun();
		}
	}
}

void MainScene::SpawnHazard() {
	float X = (rand() % 10) + 1;
	float Z = 10 - X + 1;
	Hazard* hazard = HazardFactory::buildPrism(glm::vec3(X, 1.0, Z));
	hazards.push_back(std::shared_ptr<Hazard>(hazard));

	ServerPacket packet;
	packet.type = PACKET_HAZARD_SPAWN;
	packet.hazardSpawnPacket.spawnPosition = hazard->getLocalPosition();
	packet.hazardSpawnPacket.size = hazard->getSize();
	packet.hazardSpawnPacket.fallSpeed = hazard->fallSpeed;
	networkManager->SendToClients(packet);
}

void MainScene::Update(const float delta) {
	time += delta;
	EventManager::notify(FADE, &TypeParam<float>(time));
	
	auto it = hazards.begin();
	while (it != hazards.end()) {
		if ((*it)->grounded()) {
			it = hazards.erase(it);
		} else {
			++it;
		}
	}

	if (IS_SERVER) {
		if (hazards.size() <= 10) {
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

	for (auto player : players) {
		player->update();
	}
	for (auto hazard : hazards) {
		hazard->update(delta);
	}

	_done = checkDone();
}

Scene * MainScene::GetNext() {
	return new MenuScene(SERVER_IP, IS_SERVER, NUM_LOCAL, NUM_REMOTE, time);
}

void MainScene::notify(EventName eventName, Param* params) {
	switch (eventName) {
		case SPAWN_HAZARD: {
			TypeParam<Hazard*> *p = dynamic_cast<TypeParam<Hazard*> *>(params);
			Hazard* hazard = p->Param;
			hazards.push_back(std::shared_ptr<Hazard>(hazard));
			break;
		}
	}
}