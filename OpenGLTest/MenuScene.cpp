#include "MenuScene.h"

#include <string>
#include "GLFW/glfw3.h"
#include "MainScene.h"
#include "Colour.h"
#include "UI/UIManager.h"

#define BASE_Z 28

MenuScene::MenuScene() {
	_serverIP = {
			1,2,7,
			0,0,0,
			0,0,0,
			0,0,1
	};

	_isServer = true;

	floor.setLocalScale(32.0f);
	floor.setLocalPosition(glm::vec3(0, -32, 0));
	floor.addRenderable();
	floor.renderable->roughness = 0.8;
	floor.renderable->color = Colour::BLUERA;
	floor.renderable->model = MODEL_CUBE;
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(floor.renderable), false);

	_gameObjects = std::vector<GameObject>(X_INDEX_MAX);

	_gameObjects[0].setLocalScale(2.0f);
	_gameObjects[0].setLocalPosition(glm::vec3(-30, 4, BASE_Z - _isServer * 5));
	_gameObjects[0].addRenderable();
	_gameObjects[0].renderable->roughness = 0.5;
	_gameObjects[0].renderable->metallic = 1.0;
	_gameObjects[0].renderable->color = Colour::GOLD;
	_gameObjects[0].renderable->model = MODEL_CUBE_BEVEL;
	_gameObjects[0].renderable->interpolated = true;
	_gameObjects[0].updateRenderableTransforms();
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(_gameObjects[0].renderable), false);

	for (int i = 1; i < _gameObjects.size(); ++i) {
		_gameObjects[i].setLocalScale(2.0f);
		_gameObjects[i].setLocalPosition(glm::vec3(i * 5 - 30, 2, BASE_Z - _serverIP[i - 1] * 5));
		_gameObjects[i].addRenderable();
		_gameObjects[i].renderable->roughness = 0.5;
		_gameObjects[i].renderable->metallic = 1.0;
		_gameObjects[i].renderable->color = Colour::SILVER;
		_gameObjects[i].renderable->model = MODEL_CUBE_BEVEL;
		_gameObjects[i].renderable->interpolated = true;
		EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(_gameObjects[i].renderable), false);
	}

	Camera camera;
	camera.position = glm::vec3(0.0f, 64.0f, 100.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.FOV = 25.0f;
	camera.nearClip = 0.1f;
	camera.farClip = 1000.0f;
	EventManager::notify(RENDERER_SET_CAMERA, &TypeParam<Camera>(camera), false);

	DirectionalLight directionalLight;
	directionalLight.direction = glm::normalize(glm::vec3(1.0f, -0.5f, -0.25f));
	directionalLight.color = Colour::BEIGARA;
	directionalLight.nearclip = -50.0f;
	directionalLight.farclip = 50.0f;
	EventManager::notify(RENDERER_SET_DIRECTIONAL_LIGHT, &TypeParam<DirectionalLight>(directionalLight), false);

	EventManager::notify(RENDERER_SET_AMBIENT_UP, &TypeParam<glm::vec3>(glm::vec3(0.1f, 0.15f, 0.25f)), false);

	EventManager::notify(RENDERER_SET_FLOOR_COLOR, &TypeParam<glm::vec3>(floor.renderable->color), false);

	EventManager::subscribe(KEY_DOWN, this);

	//sound
	EventManager::notify(S_CLEAR, NULL, false);
	EventManager::notify(PLAY_BGM_N, &TypeParam<int>(0), false);
	printf("????");

	// grab UI components
	_UImenuScene = UIManager::GetComponentById("MenuScene");
	_UIisServer = (TextComponent*)UIManager::GetComponentById("isServerText");
	_UIserverIP = std::vector<TextComponent*>(_serverIP.size());
	for (int i = 0; i < _serverIP.size(); ++i) {
		_UIserverIP[i] = (TextComponent*)UIManager::GetComponentById("ipDigit" + std::to_string(i));
	}
	_UIipBox = UIManager::GetComponentById("ipBox");
	// set UI component values
	for (int i = 0; i < _serverIP.size(); ++i) {
		_UIserverIP[i]->SetText(std::to_string(_serverIP[i]));
	}
	_UIisServer->SetText((_isServer) ? "Server" : "client");
	_UIipBox->visible = (_isServer) ? false : true;
	_UImenuScene->visible = true;

}

MenuScene::~MenuScene() {
	EventManager::unsubscribe(KEY_DOWN, this);
	_UImenuScene->visible = false;
}

void MenuScene::Update(const float delta) {
	EventManager::notify(FIXED_UPDATE_STARTED_UPDATING_RENDERABLES, NULL, false);
	for (GameObject &gameObject : _gameObjects) {
		gameObject.updateRenderableTransforms();
	}
	EventManager::notify(FIXED_UPDATE_FINISHED_UPDATING_RENDERABLES, &TypeParam<float>(delta), false);
}

Scene * MenuScene::GetNext() {
	std::string strIP;
	strIP += std::to_string(_serverIP[0] * 100 + _serverIP[1] * 10 + _serverIP[2]);
	strIP += '.';
	strIP += std::to_string(_serverIP[3] * 100 + _serverIP[4] * 10 + _serverIP[5]);
	strIP += '.';
	strIP += std::to_string(_serverIP[6] * 100 + _serverIP[7] * 10 + _serverIP[8]);
	strIP += '.';
	strIP += std::to_string(_serverIP[9] * 100 + _serverIP[10] * 10 + _serverIP[11]);
	return new MainScene(_isServer, strIP);
}

void MenuScene::UIMoveY(int delta_y) {
	glm::vec3 pos = _gameObjects[_xIndex].getLocalPosition();
	if (_xIndex == 0) {
		_isServer = !_isServer;
		pos.z = BASE_Z - _isServer * 5;
		_UIisServer->SetText((_isServer) ? "Server" : "Client");
		_UIipBox->visible = (_isServer) ? false : true;
	} else {
		int value = _serverIP[_xIndex - 1] + delta_y;
		if (value < 0) value = 10 + value;
		if (value >= 10) value = value % 10;
		_UIserverIP[_xIndex - 1]->SetText(std::to_string(value));
		_serverIP[_xIndex - 1] = value;
		pos.z = BASE_Z - value * 5;
	}
	_gameObjects[_xIndex].setLocalPosition(pos);
}

void MenuScene::UIMoveX(int delta_x) {
	_gameObjects[_xIndex].setLocalPositionY(2.0f);
	if (_xIndex > 0) {
		_UIserverIP[_xIndex - 1]->position.y -= 16;
	}
	_xIndex += delta_x;
	if (_isServer) _xIndex = 0;
	if (_xIndex < 0) _xIndex = X_INDEX_MAX + _xIndex;
	if (_xIndex >= X_INDEX_MAX) _xIndex = _xIndex % X_INDEX_MAX;
	if (_xIndex > 0) {
		_UIserverIP[_xIndex - 1]->position.y += 16;
	}
	_gameObjects[_xIndex].setLocalPositionY(4.0f);
}

void MenuScene::notify(EventName eventName, Param * params) {
	if (eventName == KEY_DOWN) {
		TypeParam<int> *p = dynamic_cast<TypeParam<int> *>(params);
		int key = p->Param;
		switch (key) {
			case GLFW_KEY_A:
			case GLFW_KEY_LEFT:
				UIMoveX(-1);
				break;
				break;
			case GLFW_KEY_D:
			case GLFW_KEY_RIGHT:
				UIMoveX(1);
				break;
			case GLFW_KEY_W:
			case GLFW_KEY_UP:
				UIMoveY(1);
				break;
			case GLFW_KEY_S:
			case GLFW_KEY_DOWN:
				UIMoveY(-1);
				break;
			case GLFW_KEY_ENTER:
				_done = true;
			default:
				break;
		}
	}
}
