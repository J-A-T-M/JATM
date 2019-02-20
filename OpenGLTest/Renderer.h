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
		int RenderLoop();
		~Renderer();

	private:
		void draw();
		void DrawRenderable(std::shared_ptr<Renderable> renderable);
		void DrawRenderableDepthMap(std::shared_ptr<Renderable> renderable);
		void PreloadAssetBuffers();
		void CreateShaderProgram(GLuint & programLoc, const char * vertexShaderPath, const char * fragmentShaderPath);
		// Overrides ISubscriber::notify
        void notify(EventName eventName, Param* params); 
		
		float calculateInterpolationValue();
		std::chrono::time_point<std::chrono::high_resolution_clock> interp_start;
		float interp_duration;
		float interp_value;

		DirectionalLight directionalLight;
		glm::vec3 ambient_color_up;
		glm::vec3 ambient_color_down;
		Camera camera;

		std::thread renderThread;
		bool renderThreadShouldDie;

		std::list<std::shared_ptr<Renderable>> renderables;
		std::list<std::shared_ptr<Renderable>> renderables_waitList;
		std::mutex renderables_waitList_mutex;

		GLuint mainProgram, VAO;
		const GLuint WIDTH = 1600;
		const GLuint HEIGHT = 900;

		GLuint shadowProgram, depthMap, depthMapFBO;
		const GLuint SHADOW_WIDTH = 1024;
		const GLuint SHADOW_HEIGHT = 1024;

		GLuint uiProgram;
};
