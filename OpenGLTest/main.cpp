#include <Windows.h>
#include <iostream>
#include "Renderer.h"
#include "Renderable.h"

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

	std::cout << "goodbye world\n";
	std::cin.get();
	return 0;
}