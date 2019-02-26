#include "Renderer.h"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>

#include "AssetLoader.h"
#include "Enums.h"
#include "InputManager.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

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
	UNIFORM_DEPTHMAP_SAMPLER,
	UNIFORM_SHADOW_MODEL_MATRIX,
	UNIFORM_SHADOW_LIGHTSPACE_MATRIX,
	UNIFORM_UI_VIEWPROJECTION_MATRIX,
	UNIFORM_UI_MODEL_MATRIX,
	UNIFORM_UI_MATERIAL_COLOR,
	UNIFORM_SAMPLES,
	UNIFORM_NUM_SAMPLES,
	NUM_UNIFORMS
};
GLuint uniforms[NUM_UNIFORMS];

glm::mat4 Renderer::CalculateModelMatrix(std::shared_ptr<Renderable> renderable) {
	glm::mat4 m = glm::mat4(1.0);
	if (renderable->interpolated) {
		glm::vec3 interpolated_position = glm::mix(renderable->previousPosition, renderable->position, interp_value);
		glm::quat interpolated_rotation = glm::slerp(renderable->previousRotation, renderable->rotation, interp_value);
		float interpolated_scale = glm::mix(renderable->previousScale, renderable->scale, interp_value);
		m = glm::translate(m, interpolated_position);
		m = m * (glm::mat4)interpolated_rotation;
		m = glm::scale(m, glm::vec3(interpolated_scale));
	} else {
		m = glm::translate(m, renderable->position);
		m = m * (glm::mat4)renderable->rotation;
		m = glm::scale(m, glm::vec3(renderable->scale));
	}
	return m;
}

void Renderer::DrawRenderable(std::shared_ptr<Renderable> renderable) {
	Model* model = &AssetLoader::models[renderable->model];
	Texture* texture = &AssetLoader::textures[renderable->texture];

	glBindBuffer(GL_ARRAY_BUFFER, model->positionLoc);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, model->UVLoc);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, model->normalLoc);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), BUFFER_OFFSET(0));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->elementLoc);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->loc);

	glm::mat4 m = CalculateModelMatrix(renderable);
	glUniformMatrix4fv(uniforms[UNIFORM_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(m));
	glUniform4fv(uniforms[UNIFORM_MATERIAL_COLOR], 1, glm::value_ptr(glm::convertSRGBToLinear(renderable->color)));
	glUniform1i(uniforms[UNIFORM_MATERIAL_FULLBRIGHT], renderable->fullBright);
	glUniform1f(uniforms[UNIFORM_MATERIAL_ROUGHNESS], glm::max(renderable->roughness, 0.01f));
	glUniform1f(uniforms[UNIFORM_MATERIAL_METALLIC], renderable->metallic);
	glUniform1f(uniforms[UNIFORM_MATERIAL_F0], renderable->f0);

	glDrawElements(GL_TRIANGLES, model->elements.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}

void Renderer::DrawRenderableDepthMap(std::shared_ptr<Renderable> renderable) {
	Model* model = &AssetLoader::models[renderable->model];
	Texture* texture = &AssetLoader::textures[renderable->texture];

	glBindBuffer(GL_ARRAY_BUFFER, model->positionLoc);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), BUFFER_OFFSET(0));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->elementLoc);

	glm::mat4 m = CalculateModelMatrix(renderable);
	glUniformMatrix4fv(uniforms[UNIFORM_SHADOW_MODEL_MATRIX], 1, GL_FALSE, glm::value_ptr(m));

	glDrawElements(GL_TRIANGLES, model->elements.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}

void Renderer::Draw() {
	// draw shadow stuff
	glm::mat4 light_view = glm::lookAt(
		glm::vec3(0, 0, 0),
		directionalLight.direction,
		glm::vec3(0.01, 1, 0)
	);
	glm::mat4 light_projection = glm::ortho(-50.0f, 50.0f, -32.0f, 32.0f, directionalLight.nearclip, directionalLight.farclip);
	glm::mat4 light_viewProjection = light_projection * light_view;
	glUseProgram(depthMapProgram);
	glUniformMatrix4fv(uniforms[UNIFORM_SHADOW_LIGHTSPACE_MATRIX], 1, GL_FALSE, glm::value_ptr(light_viewProjection));
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);
	for (auto renderable : renderables) {
		DrawRenderableDepthMap(renderable);
	}

	// draw 3d stuff
	glm::mat4 camera_view = glm::lookAt(
		camera.position,
		camera.target,
		glm::vec3(0, 1, 0)
	);
	glm::mat4 camera_projection = glm::perspective(glm::radians(camera.FOV), (GLfloat)windowWidth / (GLfloat)windowHeight, camera.nearClip, camera.farClip);
	glm::mat4 camera_viewProjection = camera_projection * camera_view;
	glUseProgram(depthMapProgram);
	glUniformMatrix4fv(uniforms[UNIFORM_SHADOW_LIGHTSPACE_MATRIX], 1, GL_FALSE, glm::value_ptr(camera_viewProjection));
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);
	for (auto renderable : renderables) {
		DrawRenderableDepthMap(renderable);
	}

	glm::vec3 camera_lightDir = camera_view * glm::vec4(directionalLight.direction, 0.0f);
	glm::vec3 camera_upDir = camera_view * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	glUseProgram(mainProgram);
	glUniformMatrix4fv(uniforms[UNIFORM_VIEW_MATRIX], 1, GL_FALSE, glm::value_ptr(camera_view));
	glUniformMatrix4fv(uniforms[UNIFORM_PROJECTION_MATRIX], 1, GL_FALSE, glm::value_ptr(camera_projection));
	glUniformMatrix4fv(uniforms[UNIFORM_LIGHTSPACE_MATRIX], 1, GL_FALSE, glm::value_ptr(light_viewProjection));
	glUniform3fv(uniforms[UNIFORM_UP], 1, glm::value_ptr(camera_upDir));
	glUniform3fv(uniforms[UNIFORM_AMBIENT_COLOR_UP], 1, glm::value_ptr(glm::convertSRGBToLinear(ambient_color_up)));
	glUniform3fv(uniforms[UNIFORM_AMBIENT_COLOR_DOWN], 1, glm::value_ptr(glm::convertSRGBToLinear(ambient_color_down)));
	glUniform3fv(uniforms[UNIFORM_LIGHT_COLOR], 1, glm::value_ptr(glm::convertSRGBToLinear(directionalLight.color)));
	glUniform3fv(uniforms[UNIFORM_LIGHT_DIRECTION], 1, glm::value_ptr(camera_lightDir));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);
	for (auto renderable : renderables) {
		DrawRenderable(renderable);
	}

	/* draw 2d stuff
	glUseProgram(uiProgram);
	glm::mat4 ui_viewProjection = glm::ortho(0.0f, (GLfloat)WIDTH, 0.0f, (GLfloat)HEIGHT, -100.0f, 100.0f);
	glUniformMatrix4fv(uniforms[UNIFORM_UI_VIEWPROJECTION_MATRIX], 1, GL_FALSE, glm::value_ptr(ui_viewProjection));
	*/
}

void Renderer::PreloadAssetBuffers() {
	for (int i = 0; i < NUM_MODELS; i++) {
		Model* model = &AssetLoader::models[i];
		glGenBuffers(1, &model->positionLoc);
		glGenBuffers(1, &model->UVLoc);
		glGenBuffers(1, &model->normalLoc);
		glGenBuffers(1, &model->elementLoc);
		glBindBuffer(GL_ARRAY_BUFFER, model->positionLoc);
		glBufferData(GL_ARRAY_BUFFER, model->positions.size() * sizeof(glm::vec3), model->positions.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, model->UVLoc);
		glBufferData(GL_ARRAY_BUFFER, model->UVs.size() * sizeof(glm::vec2), model->UVs.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, model->normalLoc);
		glBufferData(GL_ARRAY_BUFFER, model->normals.size() * sizeof(glm::vec3), model->normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->elementLoc);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->elements.size() * sizeof(GLuint), model->elements.data(), GL_STATIC_DRAW);
	}
	for (int i = 0; i < NUM_TEXTURES; i++) {
		Texture* texture = &AssetLoader::textures[i];
		glGenTextures(1, &texture->loc);
		glBindTexture(GL_TEXTURE_2D, texture->loc);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data.data());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
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

float Renderer::calculateInterpolationValue() {
	auto curr_time = std::chrono::high_resolution_clock::now();
	float elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>>(curr_time - interp_start).count();
	return elapsed_time / interp_duration;
}

bool Renderer::Done() {
	return renderThreadDone;
}

int Renderer::Init() {
	//Setup GLFW
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	window = glfwCreateWindow(windowWidth, windowHeight, "Rocks Fall, Everyone Dies", nullptr, nullptr);

	//Ensure window was created
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

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
	CreateShaderProgram(depthMapProgram, "./ShadowMap", "./ShadowMap");
	CreateShaderProgram(uiProgram, "./UI", "./UI");

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// shadow stuff
	glGenFramebuffers(1, &shadowMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// ssao stuff
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
	uniforms[UNIFORM_SHADOW_MODEL_MATRIX] = glGetUniformLocation(depthMapProgram, "model");
	uniforms[UNIFORM_SHADOW_LIGHTSPACE_MATRIX] = glGetUniformLocation(depthMapProgram, "viewProjection");
	// uniforms for SSAO
	uniforms[UNIFORM_DEPTHMAP_SAMPLER] = glGetUniformLocation(mainProgram, "u_depthMapSampler");
	uniforms[UNIFORM_SAMPLES] = glGetUniformLocation(mainProgram, "u_samples");
	uniforms[UNIFORM_NUM_SAMPLES] = glGetUniformLocation(mainProgram, "u_numSamples");
	// uniforms for 2d stuff
	uniforms[UNIFORM_UI_MODEL_MATRIX] = glGetUniformLocation(uiProgram, "model");
	uniforms[UNIFORM_UI_VIEWPROJECTION_MATRIX] = glGetUniformLocation(uiProgram, "viewProjection");
	uniforms[UNIFORM_UI_MATERIAL_COLOR] = glGetUniformLocation(uiProgram, "u_color");

	glUseProgram(mainProgram);
	// binds UNIFORM_SHADOWMAP_SAMPLER to GL_TEXTURE1
	glUniform1i(uniforms[UNIFORM_SHADOWMAP_SAMPLER], 1);
	// binds UNIFORM_DEPTHMAP_SAMPLER to GL_TEXTURE2
	glUniform1i(uniforms[UNIFORM_DEPTHMAP_SAMPLER], 2);
	// binds SSAO kern
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;
	generator.seed(2);
	int num_samples = 8;
	for (unsigned int i = 0; i < num_samples; ++i) {
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		if (sample.z > 0.25) {
			float scale = (float)i / (float)num_samples;
			scale = glm::mix(0.1f, 1.0f, scale * scale);
			ssaoKernel.push_back(sample * scale);
		} else {
			--i;
		}
	}
	glUniform3fv(uniforms[UNIFORM_SAMPLES], num_samples, glm::value_ptr(ssaoKernel[0]));
	glUniform1i(uniforms[UNIFORM_NUM_SAMPLES], num_samples);

	// wireframe mode if we want to enable it for debugging
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// use GPU linear to SRGB conversion
	glEnable(GL_FRAMEBUFFER_SRGB);

	// use depth culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// use alpha for transperancy
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// use multisample anti-aliasing
	glEnable(GL_MULTISAMPLE);

	// set opengl to swap framebuffer every # screen refreshes
	glfwSwapInterval(1);

	glm::vec3 linear_ambient_color_up = glm::convertSRGBToLinear(ambient_color_up);
	glClearColor(linear_ambient_color_up.r, linear_ambient_color_up.g, linear_ambient_color_up.b, 1.0f);

	PreloadAssetBuffers();
	InputManager::registerInputCallbacks(window);

	return EXIT_SUCCESS;
}

int Renderer::RenderLoop() {
	if (Init() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	while (!glfwWindowShouldClose(window) && !renderThreadDone) {
		//Check for events like key pressed, mouse moves, etc.
		glfwPollEvents();

		renderables_mutex.lock();
		// remove renderables referenced only by renderables list
		for (auto it = renderables.begin(); it != renderables.end();) {
			if (it->use_count() == 1) {
				it = renderables.erase(it);
			} else {
				++it;
			}
		}

		//draw
		interp_value = calculateInterpolationValue();
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		if (windowWidth != 0 && windowHeight != 0) {
			Draw();
		}
		renderables_mutex.unlock();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	renderThreadDone = true;
	return EXIT_SUCCESS;
}

void Renderer::notify(EventName eventName, Param* params) {
    switch (eventName) {
		case RENDERER_ADD_TO_RENDERABLES: {
			TypeParam<std::shared_ptr<Renderable>> *p = dynamic_cast<TypeParam<std::shared_ptr<Renderable>> *>(params);
			renderables_mutex.lock();
			renderables.push_back(p->Param);
			renderables_mutex.unlock();
			break;
		}
		
        case RENDERER_SET_CAMERA: {
            TypeParam<Camera> *p = dynamic_cast<TypeParam<Camera> *>(params);
            camera = p->Param;
            break;
        }

		case RENDERER_SET_DIRECTIONAL_LIGHT: {
			TypeParam<DirectionalLight> *p = dynamic_cast<TypeParam<DirectionalLight> *>(params);
			directionalLight = p->Param;
			break;
		}

		case RENDERER_SET_AMBIENT_UP: {
			TypeParam<glm::vec3> *p = dynamic_cast<TypeParam<glm::vec3> *>(params);
			ambient_color_up = p->Param;
			break;
		}

		case RENDERER_SET_AMBIENT_DOWN: {
			TypeParam<glm::vec3> *p = dynamic_cast<TypeParam<glm::vec3> *>(params);
			ambient_color_down = p->Param;
			break;
		}

		case FIXED_UPDATE_STARTED_UPDATING_RENDERABLES: {
			renderables_mutex.lock();
			break;
		}

		case FIXED_UPDATE_FINISHED_UPDATING_RENDERABLES: {
			TypeParam<float> *p = dynamic_cast<TypeParam<float> *>(params);
			interp_duration = p->Param;
			interp_start = std::chrono::high_resolution_clock::now();
			renderables_mutex.unlock();
			break;
		}

		default:
			break;
    }
}

Renderer::Renderer() {
	renderThreadDone = false;
	renderThread = std::thread(&Renderer::RenderLoop, this);
    EventManager::subscribe(RENDERER_ADD_TO_RENDERABLES, this);
	EventManager::subscribe(RENDERER_SET_CAMERA, this);
	EventManager::subscribe(RENDERER_SET_DIRECTIONAL_LIGHT, this);
	EventManager::subscribe(RENDERER_SET_AMBIENT_UP, this);
	EventManager::subscribe(RENDERER_SET_AMBIENT_DOWN, this);
	EventManager::subscribe(FIXED_UPDATE_STARTED_UPDATING_RENDERABLES, this);
	EventManager::subscribe(FIXED_UPDATE_FINISHED_UPDATING_RENDERABLES, this);
}

Renderer::~Renderer() {
	renderThreadDone = true;
	renderThread.join();
	EventManager::unsubscribe(RENDERER_ADD_TO_RENDERABLES, this);
	EventManager::unsubscribe(RENDERER_SET_CAMERA, this);
	EventManager::unsubscribe(RENDERER_SET_DIRECTIONAL_LIGHT, this);
	EventManager::unsubscribe(RENDERER_SET_AMBIENT_UP, this);
	EventManager::unsubscribe(RENDERER_SET_AMBIENT_DOWN, this);
	EventManager::unsubscribe(FIXED_UPDATE_STARTED_UPDATING_RENDERABLES, this);
	EventManager::unsubscribe(FIXED_UPDATE_FINISHED_UPDATING_RENDERABLES, this);
}