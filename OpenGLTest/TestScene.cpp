#include "TestScene.h"

std::vector<std::shared_ptr<GameObject>> gameObjects;

TestScene::TestScene() {
	floor.setSize(32.0f);
	floor.setLocalPosition(glm::vec3(0, -32, 0));
	floor.addRenderable(Colour::BEIGE, MODEL_CUBE, TEXTURE_NONE, 0.8f, 0.0f);
	floor.renderable->normalMap = TEXTURE_NORMAL_BRICKS;
	floor.renderable->dispMap = TEXTURE_DISP_BRICKS;

	int MAX_X = 5;
	int MAX_Y = 5;
	for (int y = 0; y < 7; ++y) {
		for (int x = 0; x < 7; ++x) {
			std::shared_ptr<GameObject> gameObject = std::make_shared<GameObject>();
			gameObject->setLocalPosition(glm::vec3(8 * x - 24, 3.0f, 8 * y - 24));
			gameObject->setLocalScale(2.0f);
			gameObject->addRenderable(Colour::FUCSHIA, MODEL_CUBE_BEVEL, TEXTURE_NONE, 1.0 - y / 6.0f, x / 6.0f);
			gameObject->renderable->normalMap = TEXTURE_NORMAL_BRICKS;
			gameObject->renderable->dispMap = TEXTURE_DISP_BRICKS;
			gameObjects.push_back(gameObject);
		}
	}
	
	camera = std::make_shared<Camera>();
	camera->newPosition = glm::vec3(0.0f, 64.0f, 100.0f);
	camera->position = glm::vec3(0.0f, 64.0f, 100.0f);
	camera->target = glm::vec3(0.0f, -16.0f, 0.0f);
	camera->FOV = 60.0f;
	camera->nearClip = 0.1f;
	camera->farClip = 1000.0f;

	DirectionalLight directionalLight;
	directionalLight.direction = glm::normalize(glm::vec3(1.0f, -0.5f, -0.25f));
	directionalLight.color = Colour::BEIGARA;
	directionalLight.nearclip = -50.0f;
	directionalLight.farclip = 50.0f;

	EventManager::notify(RENDERER_SET_CAMERA, &TypeParam<std::shared_ptr<Camera>>(camera));
	EventManager::notify(RENDERER_SET_DIRECTIONAL_LIGHT, &TypeParam<DirectionalLight>(directionalLight));
	EventManager::notify(RENDERER_SET_AMBIENT_UP, &TypeParam<glm::vec3>(Colour::BROWN));
	EventManager::notify(RENDERER_SET_FLOOR_COLOR, &TypeParam<glm::vec3>(floor.renderable->color));
}

TestScene::~TestScene() {
}

double time = 0.0;
void TestScene::Update(const float delta) {
	time += delta * 0.25;
	camera->newPosition = glm::vec3(sin(time) * 48.0f, 24.0f, cos(time) * 48.0f);
}

Scene * TestScene::GetNext() {
	return new TestScene();
}

void TestScene::notify(EventName eventName, Param * params) {
}
