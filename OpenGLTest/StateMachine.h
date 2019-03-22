#pragma once
#include "Scene.h"
class StateMachine {
	public:
		StateMachine(Scene* scene);
		~StateMachine();
		bool Done();
		void Update(const float delta);
	private:
		bool _done;
		Scene* _scene;
};
