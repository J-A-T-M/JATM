#include "Network.h"
#include <Windows.h>
#include <iostream>
#include "Renderer.h"
#include "Renderable.h"
#include "AssetLoader.h"
#include "Input.h"

#define GLEW_STATIC
#include <GLEW/glew.h>
#include <glm/glm.hpp>

#include "Enums.h"

GLFWwindow* window;
std::mutex mtx;
std::condition_variable cv;

Renderer *renderer;
Input inputHandler;

//Renderable *tempPlayerPointer;

std::vector<std::shared_ptr<Renderable>> playerSharedPointers;
std::vector<Renderable*> playerList;

bool isNetworked = true;
bool isServer = true;

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	switch (key) {
	case GLFW_KEY_LEFT:
	case GLFW_KEY_A:
		if (action == GLFW_PRESS)
		{
			//TypeParam<bool> param(true);
			//EventManager::notify(PLAYER_LEFT, &param, false);
			//EventManager::notify(AIM_LEFT, &param, false);
			printf("A pressed\n");
			
			if (isNetworked) {
				char msgBuffer[] = "a";
				ClientSendMessage(msgBuffer);
			}
			else {
				//tempPlayerPointer->position.x--;
			}
		}
		if (action == GLFW_RELEASE)
		{
			//printf("A Released\n");
		}
		break;
	case GLFW_KEY_RIGHT:
	case GLFW_KEY_D:
		if (action == GLFW_PRESS)
		{
			//printf("D pressed\n");
			if (isNetworked) {
				char msgBuffer[] = "d";
				ClientSendMessage(msgBuffer);
			}
			else {
				//tempPlayerPointer->position.x++;
			}
		}
		if (action == GLFW_RELEASE)
		{

		}
		break;

	case GLFW_KEY_UP:
	case GLFW_KEY_W:
		if (action == GLFW_PRESS)
		{
			if (isNetworked) {
				char msgBuffer[] = "w";
				ClientSendMessage(msgBuffer);
			}
			else {
				//tempPlayerPointer->position.z--;
			}
		}
		break;

	case GLFW_KEY_DOWN:
	case GLFW_KEY_S:
		if (action == GLFW_PRESS)
		{
			if (isNetworked) {
				char msgBuffer[] = "s";
				ClientSendMessage(msgBuffer);
			}
			else {
				//tempPlayerPointer->position.z++;
			}
		}
		break;

	case GLFW_KEY_Q:
		if (action == GLFW_PRESS)
		{
			if (isNetworked) {
				char msgBuffer[] = "q";
				ClientSendMessage(msgBuffer);
			}
			else {
				//tempPlayerPointer->position.z++;
			}
		}
		break;

	case GLFW_KEY_E:
		if (action == GLFW_PRESS)
		{
			if (isNetworked) {
				char msgBuffer[] = "e";
				ClientSendMessage(msgBuffer);
			}
			else {
				//tempPlayerPointer->position.z++;
			}
		}
		break;

	case GLFW_KEY_SPACE:
		if (action == GLFW_PRESS)
		{
			printf("Space pressed\n");
		}
		if (action == GLFW_RELEASE)
		{

		}
		break;
	case GLFW_KEY_F:
		if (action == GLFW_PRESS)
		{

		}
	default:

		break;
	}

}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) //GLFW_RELEASE is the other possible state.
	{
		//printf("%lf %lf\n", xpos, ypos);
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) //GLFW_RELEASE is the other possible state.
	{
		//printf("Right mouse button released\n");
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) //GLFW_RELEASE is the other possible state.
	{
		//printf("left mouse button clicked at: %lf %lf\n", xpos, ypos);
	}
}

int __cdecl main() {

	//Server setup
	
	if (isNetworked) {
		if (isServer) {
			std::thread serverThread = std::thread(ServerLoop); 
			serverThread.detach();
		}
		else {
			std::thread clientThread = std::thread(ClientLoop);
			clientThread.detach();
		}
	}
	
	AssetLoader::preloadAssets();
	renderer = new Renderer();
	std::unique_lock<std::mutex> lck(mtx);
	std::thread renderThread = std::thread(&Renderer::RenderLoop, renderer);

	for (int i = 0;i < MAX_CLIENTS;i++) {

		Renderable *playerRenderable = new Renderable();
		playerRenderable->metallic = 1.0f;
		playerRenderable->position = glm::vec3(0.0, 1.0, i * 3.0);
		playerRenderable->scale = glm::vec3(2.0f);
		playerRenderable->color = glm::vec4(1.000, 0.766, 0.336, 1.0);
		playerRenderable->model = MODEL_SPHERE;
		std::shared_ptr<Renderable> player_renderable_ptr(playerRenderable);
		EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(player_renderable_ptr), false);

		playerSharedPointers.push_back(player_renderable_ptr);
		playerList.push_back(playerRenderable);
	}
	
	Renderable plastic_sphere;
	plastic_sphere.metallic = 0.0f;
	plastic_sphere.position = glm::vec3(-10.0, 1.0, 0.0);
	plastic_sphere.scale = glm::vec3(2.0f);
	plastic_sphere.color = glm::vec4(0.5, 1.0, 0.1, 1.0);
	plastic_sphere.model = MODEL_SPHERE;
	std::shared_ptr<Renderable> plastic_sphere_ptr(&plastic_sphere);
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(plastic_sphere_ptr), false);

	Renderable gold_sphere;
	gold_sphere.metallic = 1.0f;
	gold_sphere.position = glm::vec3(0.0, 1.0, 0.0);
	gold_sphere.scale = glm::vec3(2.0f);
	gold_sphere.color = glm::vec4(1.000, 0.766, 0.336, 1.0);
	gold_sphere.model = MODEL_SPHERE;
	std::shared_ptr<Renderable> gold_sphere_ptr(&gold_sphere);
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(gold_sphere_ptr), false);
	//tempPlayerPointer = &gold_sphere;

	Renderable copper_sphere;
	copper_sphere.metallic = 1.0f;
	copper_sphere.position = glm::vec3(10.0, 1.0, 0.0);
	copper_sphere.scale = glm::vec3(2.0f);
	copper_sphere.color = glm::vec4(0.955, 0.637, 0.538, 1.0);
	copper_sphere.model = MODEL_SPHERE;
	std::shared_ptr<Renderable> copper_sphere_ptr(&copper_sphere);
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(copper_sphere_ptr), false);
	


	Renderable floor;
	floor.roughness = 0.8;
	floor.color = glm::vec4(0.25, 0.25, 0.25, 1.0);
	floor.scale = glm::vec3(64);
	floor.position = glm::vec3(0, -33, 0);
	floor.model = MODEL_CUBE;
	std::shared_ptr<Renderable> ptr2(&floor);
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(ptr2), false);


	renderer->light_direction.y = -0.5;

	cv.wait(lck);
	inputHandler.setInputCallbacks(window, KeyCallback, mouse_button_callback);	//setup input handling

	
	for (int i = 0;; i++) { //temp network code, logic will be eventually moved elsewhere

		Sleep(16);

		
		if (isNetworked) {

			for (int id = 0;id < MAX_CLIENTS;id++) {

				playerList[id]->position.x = serverPlayersData.players[id].x;
				playerList[id]->position.z = serverPlayersData.players[id].z;

			}

		}

		renderer->light_direction.z = sin(i * 0.01);
		renderer->light_direction.x = cos(i * 0.01);
		copper_sphere.model = (ModelEnum)( (i / 30) % NUM_MODELS);
	}

	std::cout << "goodbye world\n";
	std::cin.get();
	return 0;
}