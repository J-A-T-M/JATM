#include "Renderer.h"

#include <algorithm>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum {
	UNIFORM_MODEL_MATRIX,
	UNIFORM_VIEW_MATRIX,
	UNIFORM_PROJECTION_MATRIX,
	UNIFORM_LIGHTSPACE_MATRIX,
	UNIFORM_MATERIAL_COLOR,
	UNIFORM_MATERIAL_FULLBRIGHT,
	UNIFORM_MATERIAL_ROUGHNESS,
	UNIFORM_MATERIAL_METALLIC,
	UNIFORM_MATERIAL_F0,
	UNIFORM_UP,
	UNIFORM_AMBIENT_COLOR_UP,
	UNIFORM_AMBIENT_COLOR_DOWN,
	UNIFORM_LIGHT_COLOR,
	UNIFORM_LIGHT_DIRECTION,
	UNIFORM_SHADOWMAP_SAMPLER,
	UNIFORM_SHADOW_MODEL_MATRIX,
	UNIFORM_SHADOW_LIGHTSPACE_MATRIX,
	UNIFORM_UI_VIEWPROJECTION_MATRIX,
	UNIFORM_UI_MODEL_MATRIX,
	UNIFORM_UI_MATERIAL_COLOR,
	NUM_UNIFORMS
};
GLuint uniforms[NUM_UNIFORMS];

void Renderer::DrawRenderable(std::shared_ptr<Renderable> renderable) {
	glBindBuffer(GL_ARRAY_BUFFER, renderable->model.positionLoc);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, renderable->model.UVLoc);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, renderable->model.normalLoc);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable->model.elementLoc);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderable->texture.loc);

	glm::mat4 m = glm::mat4(1.0);
	m = glm::translate(m, renderable->getPosition3());
	m = glm::rotate(m, renderable->rotation.z * (float)M_PI / 180.0f, glm::vec3(0, 0, 1));
	m = glm::rotate(m, renderable->rotation.y * (float)M_PI / 180.0f, glm::vec3(0, 1, 0));
	m = glm::rotate(m, renderable->rotation.x * (float)M_PI / 180.0f, glm::vec3(1, 0, 0));
	m = glm::scale(m, renderable->scale);

	glUniformMatrix4fv(uniforms[UNIFORM_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(m));
	glUniform4fv(uniforms[UNIFORM_MATERIAL_COLOR], 1, glm::value_ptr(glm::convertSRGBToLinear(renderable->color)));
	glUniform1i(uniforms[UNIFORM_MATERIAL_FULLBRIGHT], renderable->fullBright);
	glUniform1f(uniforms[UNIFORM_MATERIAL_ROUGHNESS], renderable->roughness);
	glUniform1f(uniforms[UNIFORM_MATERIAL_METALLIC], renderable->metallic);
	glUniform1f(uniforms[UNIFORM_MATERIAL_F0], renderable->f0);

	glDrawElements(GL_TRIANGLES, renderable->model.elements.size(), GL_UNSIGNED_INT, (void*)0);
}

void Renderer::DrawRenderable_ShadowMap(std::shared_ptr<Renderable> renderable) {
	glBindBuffer(GL_ARRAY_BUFFER, renderable->model.positionLoc);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable->model.elementLoc);

	glm::mat4 m = glm::mat4(1.0);
	m = glm::translate(m, renderable->getPosition3());
	m = glm::rotate(m, renderable->rotation.z * (float)M_PI / 180.0f, glm::vec3(0, 0, 1));
	m = glm::rotate(m, renderable->rotation.y * (float)M_PI / 180.0f, glm::vec3(0, 1, 0));
	m = glm::rotate(m, renderable->rotation.x * (float)M_PI / 180.0f, glm::vec3(1, 0, 0));
	m = glm::scale(m, renderable->scale);

	glUniformMatrix4fv(uniforms[UNIFORM_SHADOW_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(m));

	glDrawElements(GL_TRIANGLES, renderable->model.elements.size(), GL_UNSIGNED_INT, (void*)0);
}

void Renderer::draw() {
	// draw shadow stuff
	glm::mat4 light_view = glm::lookAt(
		glm::vec3(0, 0, 0),
		light_direction,
		glm::vec3(0.01, 1, 0)
	);
	glm::mat4 light_projection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, light_nearclip, light_farclip);
	glm::mat4 light_viewProjection = light_projection * light_view;
	glUseProgram(shadowProgram);
	glUniformMatrix4fv(uniforms[UNIFORM_SHADOW_LIGHTSPACE_MATRIX], 1, GL_FALSE, glm::value_ptr(light_viewProjection));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);
	for (auto renderable : renderables) {
		DrawRenderable_ShadowMap(renderable);
	}

	// draw 3d stuff
	glm::mat4 view = glm::lookAt(
		cameraPosition,
		cameraTarget,
		glm::vec3(0, 1, 0)
	);
	glm::mat4 projection = glm::perspective(cameraFOV * (float)M_PI / 180.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, nearClip, farClip);
	glm::vec3 lightDir = view * glm::vec4(light_direction, 0.0f);
	glm::vec3 upDir = view * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	glUseProgram(mainProgram);
	glUniformMatrix4fv(uniforms[UNIFORM_VIEW_MATRIX], 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(uniforms[UNIFORM_PROJECTION_MATRIX], 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(uniforms[UNIFORM_LIGHTSPACE_MATRIX], 1, GL_FALSE, glm::value_ptr(light_viewProjection));
	glUniform3fv(uniforms[UNIFORM_UP], 1, glm::value_ptr(upDir));
	glUniform3fv(uniforms[UNIFORM_AMBIENT_COLOR_UP], 1, glm::value_ptr(glm::convertSRGBToLinear(ambient_color_up)));
	glUniform3fv(uniforms[UNIFORM_AMBIENT_COLOR_DOWN], 1, glm::value_ptr(glm::convertSRGBToLinear(ambient_color_down)));
	glUniform3fv(uniforms[UNIFORM_LIGHT_COLOR], 1, glm::value_ptr(glm::convertSRGBToLinear(light_color)));
	glUniform3fv(uniforms[UNIFORM_LIGHT_DIRECTION], 1, glm::value_ptr(lightDir));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glViewport(0, 0, WIDTH, HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);
	for (auto renderable : renderables) {
		DrawRenderable(renderable);
	}

	// draw 2d stuff
	glUseProgram(uiProgram);
	glm::mat4 ui_viewProjection = glm::ortho(0.0f, (GLfloat)WIDTH, 0.0f, (GLfloat)HEIGHT, -100.0f, 100.0f);
	glUniformMatrix4fv(uniforms[UNIFORM_UI_VIEWPROJECTION_MATRIX], 1, GL_FALSE, glm::value_ptr(ui_viewProjection));
}

void Renderer::GenerateBuffers(std::shared_ptr<Renderable> renderable) {
	glGenBuffers(1, &renderable->model.positionLoc);
	glGenBuffers(1, &renderable->model.UVLoc);
	glGenBuffers(1, &renderable->model.normalLoc);
	glGenBuffers(1, &renderable->model.elementLoc);
	glGenTextures(1, &renderable->texture.loc);
}


void Renderer::PopulateBuffers(std::shared_ptr<Renderable> renderable) {
	glBindBuffer(GL_ARRAY_BUFFER, renderable->model.positionLoc);
	glBufferData(GL_ARRAY_BUFFER, renderable->model.positions.size() * sizeof(glm::vec3), renderable->model.positions.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, renderable->model.UVLoc);
	glBufferData(GL_ARRAY_BUFFER, renderable->model.UVs.size() * sizeof(glm::vec2), renderable->model.UVs.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, renderable->model.normalLoc);
	glBufferData(GL_ARRAY_BUFFER, renderable->model.normals.size() * sizeof(glm::vec3), renderable->model.normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable->model.elementLoc);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderable->model.elements.size() * sizeof(GLuint), renderable->model.elements.data(), GL_STATIC_DRAW);

	glBindTexture(GL_TEXTURE_2D, renderable->texture.loc);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, renderable->texture.width, renderable->texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, renderable->texture.data.data());
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Renderer::AddToRenderables(std::shared_ptr<Renderable> renderable) {
    GenerateBuffers(renderable);
    PopulateBuffers(renderable);
    renderables.push_back(renderable);
}


void Renderer::CreateShaderProgram(GLuint &programLoc, const char* vertexShaderPath, const char* fragmentShaderPath) {
	//Create shader program
	programLoc = glCreateProgram();
	Shader *vShader = new Shader(vertexShaderPath, GL_VERTEX_SHADER);
	glAttachShader(programLoc, vShader->GetShader());
	delete(vShader);
	Shader *fShader = new Shader(fragmentShaderPath, GL_FRAGMENT_SHADER);
	glAttachShader(programLoc, fShader->GetShader());
	delete(fShader);
	glLinkProgram(programLoc);

	//Check for shader program linking errors
	GLint success;
	GLchar infoLog[512];
	glGetProgramiv(programLoc, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(programLoc, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
}

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

int Renderer::RenderLoop() {
	std::unique_lock<std::mutex> lck(mtx);



	//Setup GLFW
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "The Farmi Paradox", nullptr, nullptr);

	//Ensure window was created
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	//Adjust width and height based on pixel density
	int scrnWidth, scrnHeight;
	glfwGetFramebufferSize(window, &scrnWidth, &scrnHeight);

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;

	//Check that GLEW is initialized
	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// spew OpenGL errors to stderr
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, 0x826b, 0, NULL, GL_FALSE);
	glDebugMessageCallback(MessageCallback, 0);
	glEnable(GL_DEBUG_OUTPUT);

	CreateShaderProgram(mainProgram, "./Standard", "./Standard");
	CreateShaderProgram(shadowProgram, "./ShadowMap", "./ShadowMap");
	CreateShaderProgram(uiProgram, "./UI", "./UI");

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// shadow stuff
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// uniforms for 3d stuff
	uniforms[UNIFORM_MODEL_MATRIX] = glGetUniformLocation(mainProgram, "model");
	uniforms[UNIFORM_VIEW_MATRIX] = glGetUniformLocation(mainProgram, "view");
	uniforms[UNIFORM_PROJECTION_MATRIX] = glGetUniformLocation(mainProgram, "projection");
	uniforms[UNIFORM_LIGHTSPACE_MATRIX] = glGetUniformLocation(mainProgram, "lightSpace");
	uniforms[UNIFORM_MATERIAL_FULLBRIGHT] = glGetUniformLocation(mainProgram, "u_fullBright");
	uniforms[UNIFORM_MATERIAL_COLOR] = glGetUniformLocation(mainProgram, "u_color");
	uniforms[UNIFORM_MATERIAL_ROUGHNESS] = glGetUniformLocation(mainProgram, "u_roughness");
	uniforms[UNIFORM_MATERIAL_METALLIC] = glGetUniformLocation(mainProgram, "u_metallic");
	uniforms[UNIFORM_MATERIAL_F0] = glGetUniformLocation(mainProgram, "u_f0");
	uniforms[UNIFORM_SHADOWMAP_SAMPLER] = glGetUniformLocation(mainProgram, "u_shadowMapSampler");
	uniforms[UNIFORM_UP] = glGetUniformLocation(mainProgram, "u_up");
	uniforms[UNIFORM_AMBIENT_COLOR_UP] = glGetUniformLocation(mainProgram, "u_ambientColorUp");
	uniforms[UNIFORM_AMBIENT_COLOR_DOWN] = glGetUniformLocation(mainProgram, "u_ambientColorDown");
	uniforms[UNIFORM_LIGHT_DIRECTION] = glGetUniformLocation(mainProgram, "u_lightDirection");
	uniforms[UNIFORM_LIGHT_COLOR] = glGetUniformLocation(mainProgram, "u_lightColor");
	// uniforms for shadow mapping
	uniforms[UNIFORM_SHADOW_MODEL_MATRIX] = glGetUniformLocation(shadowProgram, "model");
	uniforms[UNIFORM_SHADOW_LIGHTSPACE_MATRIX] = glGetUniformLocation(shadowProgram, "viewProjection");
	// uniforms for 2d stuff
	uniforms[UNIFORM_UI_MODEL_MATRIX] = glGetUniformLocation(uiProgram, "model");
	uniforms[UNIFORM_UI_VIEWPROJECTION_MATRIX] = glGetUniformLocation(uiProgram, "viewProjection");
	uniforms[UNIFORM_UI_MATERIAL_COLOR] = glGetUniformLocation(uiProgram, "u_color");

	glUseProgram(mainProgram);
	// binds UNIFORM_SHADOWMAP_SAMPLER to GL_TEXTURE1
	glUniform1i(uniforms[UNIFORM_SHADOWMAP_SAMPLER], 1);

	// wireframe mode if we want to enable it for debugging
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// convert linear fragment shader output to srgb automatically
	glEnable(GL_FRAMEBUFFER_SRGB);

	// don't draw polygons if they are behind other polygons
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	// use alpha for transperancy
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// set opengl to swap framebuffer every # screen refreshes
	glfwSwapInterval(1);

	glClearColor(ambient_color_up.r, ambient_color_up.g, ambient_color_up.b, 1.0f);

	lck.unlock();
	cv.notify_all();

	while (!glfwWindowShouldClose(window)) {
		//Check for events like key pressed, mouse moves, etc.
		glfwPollEvents();

		renderables_waitList_mutex.lock();
		while (renderables_waitList.size() != 0) {
			AddToRenderables(renderables_waitList.back());
			renderables_waitList.pop_back();
		}
		renderables_waitList_mutex.unlock();

		for (std::shared_ptr<Renderable> &renderable : renderables) {
			if (renderable.use_count() == 1) {
				// set unused renderable to null while itterating
				// but don't remove them yet because deletion while itterating is spooky
				renderable = NULL;
			} else if (renderable->invalidated) {
				PopulateBuffers(renderable);
				renderable->invalidated = false;
			}
		}

		// remove unused renderables
		renderables.remove(NULL);

		//draw
		draw();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}

void Renderer::notify(EventName eventName, Param* params) {
    switch (eventName) {
		case RENDERER_ADD_TO_RENDERABLES: {
			TypeParam<std::shared_ptr<Renderable>> *p = dynamic_cast<TypeParam<std::shared_ptr<Renderable>> *>(params);
			renderables_waitList_mutex.lock();
			renderables_waitList.push_back(p->Param);
			renderables_waitList_mutex.unlock();
			break;
		}
		
        case RENDERER_SET_CAMERA: {
            TypeParam<glm::vec3> *p = dynamic_cast<TypeParam<glm::vec3> *>(params);
            glm::vec3 pos = p->Param;
            cameraPosition = pos;
            cameraPosition.z /= std::tan(cameraFOV * M_PI / 360.0f);
            break;
        }
		default:
			break;
    }
}



Renderer::Renderer() {
    EventManager::subscribe(RENDERER_ADD_TO_RENDERABLES, this);
    EventManager::subscribe(RENDERER_ADD_TO_UIRENDERABLES, this);
    EventManager::subscribe(RENDERER_POPULATE_BUFFERS, this);
    EventManager::subscribe(RENDERER_REPOPULATE_BUFFERS, this);
    EventManager::subscribe(RENDERER_SET_CAMERA, this);
}

Renderer::~Renderer() {
    
}