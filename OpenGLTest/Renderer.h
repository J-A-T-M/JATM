#pragma once

#include <chrono>
#define GLEW_STATIC
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <list>
#include <mutex>

#include "EventManager.h"
#include "Renderable.h"
#include "shader.h"

class Renderer : public ISubscriber {
	public:
		Renderer();
		bool Done();
		~Renderer();

	private:
		int RenderLoop();
		int Init();
		void PreloadAssetBuffers();
		void Draw();
		void DrawRenderable(std::shared_ptr<Renderable> renderable);
		void DrawRenderableDepthMap(std::shared_ptr<Renderable> renderable);
		glm::mat4 CalculateModelMatrix(std::shared_ptr<Renderable> renderable);
		// Overrides ISubscriber::notify
		void notify(EventName eventName, Param* params);

		GLFWwindow* window;
		GLuint VAO;
		Shader* standardShader;
		Shader* depthMapShader;
		int windowWidth = 1600;
		int windowHeight = 900;

		// stuff for renderable transform interpolation
		void updateInterpolationValue();
		std::chrono::time_point<std::chrono::high_resolution_clock> interp_start;
		float interp_duration;
		float interp_value;

		DirectionalLight directionalLight;
		glm::vec3 ambient_color_up;
		glm::vec3 ambient_color_down;
		Camera camera;

		std::thread renderThread;
		bool renderThreadDone;

		std::list<std::shared_ptr<Renderable>> renderables;
		std::mutex renderables_mutex;

		GLuint shadowMap, shadowMapFBO;
		const GLuint SHADOW_SIZE = 1024;

		GLuint depthMap, depthMapFBO;
};
