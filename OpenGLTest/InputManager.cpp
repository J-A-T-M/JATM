#include "InputManager.h"


InputManager::InputManager() {

}

InputManager::~InputManager() {

}

void InputManager::setInputCallbacks(GLFWwindow* window, GLFWkeyfun func, GLFWmousebuttonfun mouseFunc) {
	glfwSetKeyCallback(window, func);
	glfwSetMouseButtonCallback(window, mouseFunc);
}

void InputManager::addKeyDownBinding(int key, const Callback& callback)
{
	keypress_Callbacks[key].push_back(callback);
}

void InputManager::addKeyRepeatBinding(int key, const Callback& callback) {
	keyrepeat_Callbacks[key].push_back(callback);
}

void InputManager::onKeyPress(int key)
{
	for (Callback& callback : keypress_Callbacks[key])
	{
		callback();
	}
}

void InputManager::onKeyRepeat(int key) {
	for (Callback& callback : keyrepeat_Callbacks[key])
	{
		callback();
	}
}