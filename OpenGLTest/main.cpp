#include <Windows.h>
#include <iostream>
#include "Renderer.h"
#include "Renderable.h"
#include "AssetLoader.h"

#define GLEW_STATIC
#include <GLEW/glew.h>
#include <glm/glm.hpp>

GLFWwindow* window;
std::mutex mtx;
std::condition_variable cv;

Renderer *renderer;

int main() {
	
	renderer = new Renderer();
	//std::unique_lock<std::mutex> lck(mtx);
	std::thread renderThread = std::thread(&Renderer::RenderLoop, renderer);

	Renderable plastic_sphere;
	plastic_sphere.metallic = 0.0f;
	plastic_sphere.position = glm::vec3(-10.0, 1.0, 0.0);
	plastic_sphere.scale = glm::vec3(2.0f);
	plastic_sphere.color = glm::vec4(0.5, 1.0, 0.1, 1.0);
	plastic_sphere.model = AssetLoader::loadModel("../assets/models/sphere.obj");
	std::shared_ptr<Renderable> plastic_sphere_ptr(&plastic_sphere);
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(plastic_sphere_ptr), false);

	Renderable gold_sphere;
	gold_sphere.metallic = 1.0f;
	gold_sphere.position = glm::vec3(0.0, 1.0, 0.0);
	gold_sphere.scale = glm::vec3(2.0f);
	gold_sphere.color = glm::vec4(1.000, 0.766, 0.336, 1.0);
	gold_sphere.model = AssetLoader::loadModel("../assets/models/sphere.obj");
	std::shared_ptr<Renderable> gold_sphere_ptr(&gold_sphere);
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(gold_sphere_ptr), false);

	Renderable copper_sphere;
	copper_sphere.metallic = 1.0f;
	copper_sphere.position = glm::vec3(10.0, 1.0, 0.0);
	copper_sphere.scale = glm::vec3(2.0f);
	copper_sphere.color = glm::vec4(0.955, 0.637, 0.538, 1.0);
	copper_sphere.model = AssetLoader::loadModel("../assets/models/sphere.obj");
	std::shared_ptr<Renderable> copper_sphere_ptr(&copper_sphere);
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(copper_sphere_ptr), false);

	Renderable floor;
	floor.roughness = 0.8;
	floor.color = glm::vec4(0.25, 0.25, 0.25, 1.0);
	floor.scale = glm::vec3(64);
	floor.position = glm::vec3(0, -33, 0);
	floor.model = AssetLoader::loadModel("../assets/models/cube.obj");
	std::shared_ptr<Renderable> ptr2(&floor);
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(ptr2), false);


	renderer->light_direction.y = -0.5;

	for (int i = 0;; i++) {
		Sleep(16);
		renderer->light_direction.z = sin(i * 0.01);
		renderer->light_direction.x = cos(i * 0.01);
	}

	std::cout << "goodbye world\n";
	std::cin.get();
	return 0;
}