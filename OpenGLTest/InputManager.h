#pragma once
#include <GLFW/glfw3.h>
#include <functional>
#include <map>
#include <vector>

#include "Enums.h"

struct Input {
	bool up;
	bool down;
	bool left;
	bool right;
};

class InputManager {
	public:
		using Callback = std::function<void()>;
		static void registerInputCallbacks(GLFWwindow* window);
		static void addKeyDownBinding(int key, const Callback& callback);
		static void addKeyRepeatBinding(int key, const Callback& callback);
		static Input const getInput(InputSourceEnum index);
		static void setInput(Input input, InputSourceEnum index);
		static void clearInputs();
	private:
		static Input inputs[NUM_INPUTS];
		static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

		static void onKeyPress(int key);
		static void onKeyRepeat(int key);

		static std::map<int, std::vector<Callback>> keypress_Callbacks;  //functions that are called on key down 
		static std::map<int, std::vector<Callback>> keyrepeat_Callbacks; //functions that are called when the key is held down
};
