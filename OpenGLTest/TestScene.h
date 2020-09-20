#pragma once
#include "Scene.h"

#include <vector>
#include "EventManager.h"
#include "GameObject.h"

class TestScene :
	public Scene {
	public:
	TestScene();
	~TestScene();
	void Update(const float delta);
	Scene* GetNext();
	void notify(EventName eventName, Param* params);

	GameObject floor;
	std::shared_ptr<Camera> camera;
};
