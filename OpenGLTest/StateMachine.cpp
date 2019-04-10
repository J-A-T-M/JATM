#include "StateMachine.h"

#include "UI/UIManager.h"

StateMachine::StateMachine(Scene* scene) {
	_scene = scene;
}

StateMachine::~StateMachine() {
	if (_scene != nullptr) {
		delete _scene;
	}
}

bool StateMachine::Done() {
	return _done;
}

void StateMachine::Update(const float delta) {
	if (_scene == nullptr) {
		_done = true;
		return;
	}

	_scene->Update(delta);

	if (_scene->Done()) {
		UIComponent *blackOverlay = UIManager::GetComponentById("BlackOverlay");
		blackOverlay->visible = true;
		Scene* nextScene = _scene->GetNext();
		delete _scene;
		_scene = nextScene;
		blackOverlay->visible = false;
	}
}
