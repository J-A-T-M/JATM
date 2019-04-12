#include "InputManager.h"
#include <iostream>
#include "EventManager.h"

using Callback = std::function<void()>;
std::map<int, std::vector<Callback>> InputManager::keypress_Callbacks;
std::map<int, std::vector<Callback>> InputManager::keyrepeat_Callbacks;
Input InputManager::inputs[NUM_INPUTS];

void InputManager::registerInputCallbacks(GLFWwindow* window) {
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
}

void InputManager::addKeyDownBinding(int key, const Callback& callback) {
	keypress_Callbacks[key].push_back(callback);
}

void InputManager::addKeyRepeatBinding(int key, const Callback& callback) {
	keyrepeat_Callbacks[key].push_back(callback);
}

void InputManager::onKeyPress(int key) {
	for (Callback& callback : keypress_Callbacks[key]) {
		callback();
	}
}

void InputManager::onKeyRepeat(int key) {
	for (Callback& callback : keyrepeat_Callbacks[key]) {
		callback();
	}
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_REPEAT) {
		onKeyRepeat(key);
		// return early for repeat 
		// basic player input can infer when held from PRESS and RELEASE
		EventManager::notify(KEY_REPEAT, &TypeParam<int>(key));
		return;
	} 
	if (action == GLFW_PRESS) {
		onKeyPress(key);
		EventManager::notify(KEY_DOWN, &TypeParam<int>(key));
	}
	if (action == GLFW_RELEASE) {
		EventManager::notify(KEY_UP, &TypeParam<int>(key));
	}

	switch (key) {
		// INPUT_LOCAL1
		case GLFW_KEY_A:
			inputs[INPUT_LOCAL1].left = action;
			break;
		case GLFW_KEY_D:
			inputs[INPUT_LOCAL1].right = action;
			break;
		case GLFW_KEY_W:
			inputs[INPUT_LOCAL1].up = action;
			break;
		case GLFW_KEY_S:
			inputs[INPUT_LOCAL1].down = action;
			break;
		// INPUT_LOCAL2
		case GLFW_KEY_LEFT:
			inputs[INPUT_LOCAL2].left = action;
			break;
		case GLFW_KEY_RIGHT:
			inputs[INPUT_LOCAL2].right = action;
			break;
		case GLFW_KEY_UP:
			inputs[INPUT_LOCAL2].up = action;
			break;
		case GLFW_KEY_DOWN:
			inputs[INPUT_LOCAL2].down = action;
			break;
		default:
			break;
	}
}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		std::cout << "RMB PRESS [" << xpos << ',' << ypos << ']' << std::endl;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		std::cout << "RMB RELEASE [" << xpos << ',' << ypos << ']' << std::endl;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		std::cout << "LMB PRESS [" << xpos << ',' << ypos << ']' << std::endl;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		std::cout << "LMB RELEASE [" << xpos << ',' << ypos << ']' << std::endl;
	}
}

Input const InputManager::getInput(InputSourceEnum index) {
	return inputs[index];
}

void InputManager::setInput(Input input, InputSourceEnum index) {
	if (index >= INPUT_CLIENT1) {
		inputs[index] = input;
	}
}

void InputManager::clearInputs() {
	for (int i = 0; i < NUM_INPUTS; ++i) {
		inputs[i] = {};
	}
}