#pragma once
//uses UIComponents, Renderables
#include <list>
#include <mutex>

#define GLEW_STATIC
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>

#include "EventManager.h"
#include "Renderable.h"


#include "shader.h"


extern std::mutex mtx;
extern std::condition_variable cv;
extern GLFWwindow *window;

class Renderer : public ISubscriber {
	public:
		Renderer();
		int RenderLoop();
		~Renderer();

		glm::vec3 cameraPosition = { 0.0f, 64.0f, 100.0f };
		glm::vec3 cameraTarget = { 0.0f, 0.0f, 0.0f };
		GLfloat cameraFOV = 25.0f;
		GLfloat nearClip = 0.1f;
		GLfloat farClip = 1000.0f;

		glm::vec3 ambient_color_up = { 0.25, 0.25, 0.25 };
		glm::vec3 ambient_color_down = { 0.0f, 0.0f, 0.0f };

		glm::vec3 light_direction = { 0.0f, -1.0f, 0.0f };
		glm::vec3 light_color = { 1.0f, 1.0f, 1.0f };
		GLfloat light_nearclip = -50.0f;
		GLfloat	light_farclip = 50.0f;
	private:
		void draw();
		void DrawRenderable(std::shared_ptr<Renderable> renderable);
		void DrawRenderableDepthMap(std::shared_ptr<Renderable> renderable);

		void PreloadAssetBuffers();
		void CreateShaderProgram(GLuint & programLoc, const char * vertexShaderPath, const char * fragmentShaderPath);

        void notify(EventName eventName, Param* params);    // Overrides ISubscriber::notify

		std::mutex renderables_waitList_mutex;
		std::list<std::shared_ptr<Renderable>> renderables_waitList;
		std::list<std::shared_ptr<Renderable>> renderables;
		const GLuint WIDTH = 1600;
		const GLuint HEIGHT = 900;
		GLuint mainProgram, VAO;
		const GLuint SHADOW_WIDTH = 1024;
		const GLuint SHADOW_HEIGHT = 1024;
		GLuint shadowProgram, depthMap, depthMapFBO;
		GLuint uiProgram;
};
