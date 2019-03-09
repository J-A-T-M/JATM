#pragma once
#include "Scene.h"

#include <memory>
#include <vector>

#include "Enums.h"
#include "EventManager.h"
#include "GameObject.h"
#include "Player.h"
#include "Renderable.h"
#include "Hazard.h"
#include "PhysicsManager.h"


class MainScene : public Scene, ISubscriber{
	public:
		MainScene(bool isServer, std::string serverIP);
		~MainScene();
		void Setup();
		void Update(const float delta);
		bool Done();
		void Cleanup();
		void SpawnHazard();

	private:
		void movePlayersBasedOnInput(const float delta);
		void sendPlayerTransforms();
		void movePlayersBasedOnNetworking();
		// Overrides ISubscriber::notify
		void notify(EventName eventName, Param* params);
		const bool IS_SERVER;
		const std::string SERVER_IP;
		float time;
		std::vector<Hazard*> hazards;
		std::vector<Player*> players;
		std::vector<InputSourceEnum> playerInputSources = { INPUT_LOCAL1, INPUT_LOCAL2, INPUT_CLIENT1, INPUT_CLIENT2, INPUT_CLIENT3, INPUT_CLIENT4, INPUT_CLIENT5, INPUT_CLIENT6 };
		std::thread networkThread;
		Camera camera;
		DirectionalLight directionalLight;
};
