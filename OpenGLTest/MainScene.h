#pragma once
#include "Scene.h"

#include <vector>

#include "Enums.h"
#include "PhysicsManager.h"
#include "GameObject.h"
#include "Player.h"
#include "Renderable.h"
#include "Network.h"


class MainScene : public Scene, ISubscriber{
	public:
		MainScene(bool isServer, std::string serverIP, int numLocal, int numRemote);
		~MainScene();
		void Update(const float delta);
		Scene* GetNext();
		
	private:
		bool checkDone();
		void movePlayersBasedOnInput(const float delta);
		void sendPlayerTransforms();
		void movePlayersBasedOnNetworking();
		void SpawnHazard();
		// Overrides ISubscriber::notify
		void notify(EventName eventName, Param* params);
		const bool IS_SERVER;
		const std::string SERVER_IP;
		const int NUM_LOCAL;
		const int NUM_REMOTE;
		float time;
		GameObject* floor;
		std::vector<Hazard*> hazards;
		std::vector<Player*> players;
		std::vector<InputSourceEnum> playerInputSources;
		Camera camera;
		DirectionalLight directionalLight;
		NetworkManager *networkManager;
};