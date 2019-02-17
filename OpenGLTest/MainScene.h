#pragma once
#include "Scene.h"

#include <memory>
#include <vector>

#include "Enums.h"
#include "EventManager.h"
#include "Renderable.h"

class MainScene : public Scene{
	public:
		void Setup();
		void Update(const float delta);
		bool Done();
		void Cleanup();
	private:
		void movePlayersBasedOnInput(const float delta);
		void movePlayersBasedOnNetworking();

		const bool isServer = true;
		float time;
		std::vector<std::shared_ptr<Renderable>> players;
		std::shared_ptr<Renderable> floor;
		std::vector<InputSourceEnum> playerInputSources = { INPUT_LOCAL1, INPUT_LOCAL2, INPUT_CLIENT1, INPUT_CLIENT2, INPUT_CLIENT3, INPUT_CLIENT4, INPUT_CLIENT5, INPUT_CLIENT6 };
		std::thread networkThread;
		Camera camera;
		DirectionalLight directionalLight;
};