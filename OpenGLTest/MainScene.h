#pragma once
#include "Scene.h"

#include <memory>
#include <vector>

#include "Enums.h"
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
		void notify(EventName eventName, Param* params);

		const bool IS_SERVER;
		const std::string SERVER_IP;
		const int NUM_LOCAL;
		const int NUM_REMOTE;

		NetworkManager *networkManager;
		float time;
		int livePlayers;
		std::shared_ptr<GameObject> floor;
		std::vector<std::shared_ptr<Hazard>> hazards;
		std::vector<std::shared_ptr<Player>> players;
		std::vector<InputSourceEnum> playerInputSources;
};