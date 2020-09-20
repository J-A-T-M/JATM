#pragma once
#include "Scene.h"

#include <vector>
#include "EventManager.h"
#include "GameObject.h"
#include "UI/TextComponent.h"
#include <array>

class MenuScene : public Scene, ISubscriber {
	public:
		MenuScene(std::string serverIP = "127.0.0.1", bool isServer = true, int numLocal = 2, int numRemote = 2, float previousTime = 0.0f);
		~MenuScene();
		void Update(const float delta);
		Scene* GetNext();
	private:
		static float _highScore;

		void UIMoveY(int delta_y);
		void UpdateGameObjectPositions();
		void UpdateUIPositions();
		void UIMoveX(int delta_x);

		static const int X_INDEX_MAX = 13;

		GameObject floor;
		std::shared_ptr<Camera> camera;
		std::array<GameObject, X_INDEX_MAX> _gameObjects;

		UIComponent* _UImenuScene;
		TextComponent* _UIisServer;
		UIComponent* _highScoreBox;
		TextComponent* _highScoreText;

		UIComponent* _ipBox;
		std::array<TextComponent*, 12> _UIserverIP;


		UIComponent* _numPlayersBox;
		TextComponent* _numLocal;
		TextComponent* _numRemote;

		std::array<int, 2> _numPlayers;
		std::array<int, 12> _serverIP;
		bool _isServer;
		

		int _xIndex;

		void notify(EventName eventName, Param* params);
};

