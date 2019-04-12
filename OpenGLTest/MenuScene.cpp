#include "MenuScene.h"

#include <sstream>
#include "GLFW/glfw3.h"
#include "MainScene.h"
#include "Colour.h"
#include "UI/UIManager.h"

#define BASE_Z 28
#define UI_HIGHLIGHT_OFFSET 30

float MenuScene::_highScore = 0.0f;

MenuScene::MenuScene(std::string serverIP, bool isServer, int numLocal, int numRemote, float previousTime) {
	if (previousTime > _highScore) {
		_highScore = previousTime;
	}

	int n;
	char c;
	std::stringstream ss(serverIP);
	for (int i = 0; i < 4; ++i) {
		if (i != 0) {
			ss >> c;
		}
		ss >> n;
		_serverIP[i * 3] = n / 100;
		_serverIP[i * 3 + 1] = n / 10 % 10;
		_serverIP[i * 3 + 2] = n % 10;
	}

	_isServer = isServer;
	_numPlayers = { numLocal, numRemote };

	floor.setSize(32.0f);
	floor.setLocalPosition(glm::vec3(0, -32, 0));
	floor.addRenderable(Colour::BLUERA, MODEL_CUBE, TEXTURE_NONE, 0.8f, 0.0f);

	_gameObjects[0].setSize(2.0f);
	_gameObjects[0].addRenderable(Colour::GOLD, MODEL_CUBE_BEVEL, TEXTURE_NONE, 0.5f, 1.0f);
	_gameObjects[0].renderable->interpolated = true;
	_gameObjects[0].updateRenderableTransforms();

	for (int i = 1; i < _gameObjects.size(); ++i) {
		_gameObjects[i].setSize(2.0f);
		_gameObjects[i].addRenderable(Colour::SILVER, MODEL_CUBE_BEVEL, TEXTURE_NONE, 0.5f, 1.0f);
		_gameObjects[i].renderable->interpolated = true;
	}

	Camera camera;
	camera.position = glm::vec3(0.0f, 64.0f, 100.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.FOV = 25.0f;
	camera.nearClip = 0.1f;
	camera.farClip = 1000.0f;


	DirectionalLight directionalLight;
	directionalLight.direction = glm::normalize(glm::vec3(1.0f, -0.5f, -0.25f));
	directionalLight.color = Colour::BEIGARA;
	directionalLight.nearclip = -50.0f;
	directionalLight.farclip = 50.0f;

	EventManager::notify(RENDERER_SET_CAMERA, &TypeParam<Camera>(camera));
	EventManager::notify(RENDERER_SET_DIRECTIONAL_LIGHT, &TypeParam<DirectionalLight>(directionalLight));
	EventManager::notify(RENDERER_SET_AMBIENT_UP, &TypeParam<glm::vec3>(glm::vec3(0.1f, 0.15f, 0.25f)));
	EventManager::notify(RENDERER_SET_FLOOR_COLOR, &TypeParam<glm::vec3>(floor.renderable->color));

	//sound
	EventManager::notify(S_CLEAR, NULL);
	EventManager::notify(PLAY_BGM_N, &TypeParam<int>(0));

	// grab UI components
	_UImenuScene = UIManager::GetComponentById("MenuScene");
	_UIisServer = (TextComponent*)UIManager::GetComponentById("isServerText");
	for (int i = 0; i < _serverIP.size(); ++i) {
		_UIserverIP[i] = (TextComponent*)UIManager::GetComponentById("ipDigit" + std::to_string(i));
	}
	_ipBox = UIManager::GetComponentById("ipBox");
	_numPlayersBox = UIManager::GetComponentById("numPlayerBox");
	_numLocal = (TextComponent*)UIManager::GetComponentById("numLocal");
	_numRemote = (TextComponent*)UIManager::GetComponentById("numRemote");
	_highScoreBox = UIManager::GetComponentById("highScoreBox");
	_highScoreText = (TextComponent*)UIManager::GetComponentById("highScoreText");
	// set UI component values
	_UImenuScene->visible = true;
	_highScoreBox->visible = (_highScore > 0.0f);
	_highScoreText->SetText(std::to_string(_highScore) + 's');
	UpdateGameObjectPositions();
	UpdateUIPositions();

	EventManager::subscribe(KEY_DOWN, this);
}

MenuScene::~MenuScene() {
	EventManager::unsubscribe(KEY_DOWN, this);
	for (int i = 0; i < _serverIP.size(); ++i) {
		_UIserverIP[i]->anchor.y = 0;
	}
	_UImenuScene->visible = false;
}

void MenuScene::Update(const float delta) {}

Scene * MenuScene::GetNext() {
	std::string strIP;
	strIP += std::to_string(_serverIP[0] * 100 + _serverIP[1] * 10 + _serverIP[2]);
	strIP += '.';
	strIP += std::to_string(_serverIP[3] * 100 + _serverIP[4] * 10 + _serverIP[5]);
	strIP += '.';
	strIP += std::to_string(_serverIP[6] * 100 + _serverIP[7] * 10 + _serverIP[8]);
	strIP += '.';
	strIP += std::to_string(_serverIP[9] * 100 + _serverIP[10] * 10 + _serverIP[11]);
	return new MainScene(_isServer, strIP, _numPlayers[0], _numPlayers[1]);
}

void MenuScene::UpdateGameObjectPositions() {
	if (_isServer) {
		_gameObjects[0].setLocalPosition(glm::vec3(-30, 2, BASE_Z - 5));
		for (int i = 1; i < _gameObjects.size(); ++i) {
			_gameObjects[i].setLocalPosition(glm::vec3(i * 5 - 30, 2, BASE_Z));
		}
		_gameObjects[1].setLocalPosition(glm::vec3(-25, 2, BASE_Z - 5 * _numPlayers[0]));
		_gameObjects[2].setLocalPosition(glm::vec3(-20, 2, BASE_Z - 5 * _numPlayers[1]));
	} else {
		_gameObjects[0].setLocalPosition(glm::vec3(-30, 2, BASE_Z));
		for (int i = 0; i < _serverIP.size(); ++i) {
			_gameObjects[i + 1].setLocalPosition(glm::vec3(i * 5 - 25, 2, BASE_Z - _serverIP[i] * 5));
		}
	}
	_gameObjects[_xIndex].setLocalPositionY(4);
}

void MenuScene::UpdateUIPositions() {
	_UIisServer->SetText(_isServer ? "Server" : "Client");
	_UIisServer->anchor.y = (_xIndex == 0) ? UI_HIGHLIGHT_OFFSET : 0;
	_numPlayersBox->visible = _isServer;
	_ipBox->visible = !_isServer;
	if (_isServer) {
		_numLocal->SetText(std::to_string(_numPlayers[0]));
		_numLocal->anchor.y = (_xIndex == 1) ? UI_HIGHLIGHT_OFFSET : 0;
		_numRemote->SetText(std::to_string(_numPlayers[1]));
		_numRemote->anchor.y = (_xIndex == 2) ? UI_HIGHLIGHT_OFFSET : 0;
	} else {
		for (int i = 0; i < _UIserverIP.size(); ++i) {
			_UIserverIP[i]->SetText(std::to_string(_serverIP[i]));
			_UIserverIP[i]->anchor.y = 0;
		}
		if (_xIndex != 0) {
			_UIserverIP[_xIndex - 1]->anchor.y = UI_HIGHLIGHT_OFFSET;
		}
	}
}

void MenuScene::UIMoveY(int delta_y) {
	EventManager::notify(PLAY_SE, &TypeParam<int>(0));
	// update values
	if (_xIndex == 0) {
		_isServer = !_isServer;
	} else {
		if (_isServer) {
			int value = _numPlayers[_xIndex - 1] + delta_y;
			if (value < 0) value = 3 + value;
			if (value >= 3) value = value % 3;
			_numPlayers[_xIndex - 1] = value;
		} else {
			int value = _serverIP[_xIndex - 1] + delta_y;
			if (value < 0) value = 10 + value;
			if (value >= 10) value = value % 10;
			_serverIP[_xIndex - 1] = value;
		}
	}
}

void MenuScene::UIMoveX(int delta_x) {
	EventManager::notify(PLAY_SE, &TypeParam<int>(0));
	// update index
	_xIndex += delta_x;
	if (_isServer) {
		if (_xIndex < 0) {
			_xIndex = 3 + _xIndex;
		}
		_xIndex %= 3;
	} else {
		if (_xIndex < 0) {
			_xIndex = X_INDEX_MAX + _xIndex;
		}
		_xIndex %= X_INDEX_MAX;
	}
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
		UpdateGameObjectPositions();
		UpdateUIPositions();
	}
}
