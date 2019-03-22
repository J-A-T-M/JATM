#pragma once
#include "Scene.h"

#include <vector>
#include "EventManager.h"
#include "GameObject.h"

class MenuScene : public Scene, ISubscriber {
	public:
		MenuScene();
		~MenuScene();
		void Update(const float delta);
		Scene* GetNext();
	private:
		void UIMoveY(int delta_y);
		void UIMoveX(int delta_x);
		float _time;

		GameObject floor;
		std::vector<GameObject> _gameObjects;

		std::vector<int> _serverIP;
		bool _isServer;
		
		const int X_INDEX_MAX = 13;
		int _xIndex;

		void notify(EventName eventName, Param* params);
};

