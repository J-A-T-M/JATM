#include "Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>
#include <random>

#include "AssetLoader.h"
#include "Enums.h"
#include "InputManager.h"
#include "UI/UIManager.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

glm::mat4 Renderer::CalculateModelMatrix(std::shared_ptr<Renderable> renderable) {
	glm::mat4 m = glm::mat4(1.0);
	if (renderable->interpolated) {
		glm::vec3 interpolated_position = glm::mix(renderable->renderPositionPrev, renderable->renderPositionCur, interp_value);
		glm::quat interpolated_rotation = glm::slerp(renderable->renderRotationPrev, renderable->renderRotationCur, interp_value);
		glm::vec3 interpolated_scale = glm::mix(renderable->renderScalePrev, renderable->renderScaleCur, interp_value);
		m = glm::translate(m, interpolated_position);
		m = m * (glm::mat4)interpolated_rotation;
		m = glm::scale(m, interpolated_scale);
	} else {
		m = glm::translate(m, renderable->renderPositionCur);
		m = m * (glm::mat4)renderable->renderRotationCur;
		m = glm::scale(m, glm::vec3(renderable->renderScaleCur));
	}
	return m;
}

void Renderer::DrawUIComponent(UIComponent* UIrenderable) {
	if (!UIrenderable->valid) {
		InitUIComponent(UIrenderable);
		UIrenderable->valid = true;
	}

	glBindBuffer(GL_ARRAY_BUFFER, UIrenderable->model.positionLoc);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, UIrenderable->model.UVLoc);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), BUFFER_OFFSET(0));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, UIrenderable->model.elementLoc);

	Texture* texture = &AssetLoader::textures[UIrenderable->texture];
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->loc);

	glm::mat4 m = glm::mat4(1.0);
	m = glm::translate(m, glm::vec3(UIrenderable->position, UIrenderable->z));
	uiShader->setMat4("model", m);
	uiShader->setVec4("u_color", glm::convertSRGBToLinear(UIrenderable->color));

	glDrawElements(GL_TRIANGLES, UIrenderable->model.elements.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
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

	glBindTexture(GL_TEXTURE_2D, texture->loc);

	glm::mat4 m = CalculateModelMatrix(renderable);
	standardShader->setMat4("model", m);
	standardShader->setVec3("u_color", glm::convertSRGBToLinear(renderable->color));
	standardShader->setBool("u_fullBright", renderable->fullBright);
	standardShader->setFloat("u_roughness", glm::max(renderable->roughness, 0.01f));
	standardShader->setFloat("u_metallic", renderable->metallic);

	glm::mat3 mn = m;
	mn = glm::inverseTranspose(mn);
	standardShader->setMat3("modelNormal", mn);

	glDrawElements(GL_TRIANGLES, model->elements.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}

void Renderer::DrawRenderableDepthMap(std::shared_ptr<Renderable> renderable) {
	Model* model = &AssetLoader::models[renderable->model];

	glBindBuffer(GL_ARRAY_BUFFER, model->positionLoc);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), BUFFER_OFFSET(0));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->elementLoc);

	glm::mat4 m = CalculateModelMatrix(renderable);
	depthMapShader->setMat4("model", m);

	glDrawElements(GL_TRIANGLES, model->elements.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}

void Renderer::Draw() {
	glClearColor(ambient_color_up.r, ambient_color_up.g, ambient_color_up.b, 1.0f);
	ambient_color_down = ambient_color_up;
	for (int i = 0; i < NUM_LIGHTS; ++i) {
		glm::vec3 L = glm::normalize(directionalLight[i].direction);
		ambient_color_down += -L.y * glm::convertSRGBToLinear(directionalLight[i].color);
	}
	ambient_color_down *= floor_color;

	glm::mat4 camera_view = glm::lookAt(camera.position, camera.target, glm::vec3(0, 1, 0));
	glm::mat4 camera_projection = glm::perspective(glm::radians(camera.FOV), (GLfloat)windowWidth / (GLfloat)windowHeight, camera.nearClip, camera.farClip);
	glm::mat4 camera_viewProjection = camera_projection * camera_view;
	glm::vec3 camera_upDir = camera_view * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

	glm::mat4 lightViewProjections[NUM_LIGHTS];
	glm::vec3 lightDirections[NUM_LIGHTS];
	glm::vec3 lightColors[NUM_LIGHTS];
	depthMapShader->use();
	for (int i = 0; i < NUM_LIGHTS; ++i) {
		glm::mat4 light_view = glm::lookAt(glm::vec3(0, 0, 0), directionalLight[i].direction, glm::vec3(0.01, 1, 0));
		glm::mat4 light_projection = glm::ortho(-50.0f, 50.0f, -32.0f, 32.0f, directionalLight[i].nearclip, directionalLight[i].farclip);
		glm::mat4 light_viewProjection = light_projection * light_view;
		glm::vec3 camera_lightDir = camera_view * glm::vec4(directionalLight[i].direction, 0.0f);
		depthMapShader->setMat4("viewProjection", light_viewProjection);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO[i]);
		glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
		glClear(GL_DEPTH_BUFFER_BIT);
		for (auto renderable : renderables) {
			DrawRenderableDepthMap(renderable);
		}
		lightViewProjections[i] = light_viewProjection;
		lightDirections[i] = camera_lightDir;
		lightColors[i] = glm::convertSRGBToLinear(directionalLight[i].color);
	}

	// draw camera depthmap
	depthMapShader->use();
	depthMapShader->setMat4("viewProjection", camera_viewProjection);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_DEPTH_BUFFER_BIT);
	for (auto renderable : renderables) {
		DrawRenderableDepthMap(renderable);
	}

	// draw 3d objects
	standardShader->use();
	standardShader->setMat4("view", camera_view);
	standardShader->setMat4("projection", camera_projection);
	standardShader->setVec3("u_up", camera_upDir);
	standardShader->setVec3("u_ambientColorUp", ambient_color_up);
	standardShader->setVec3("u_ambientColorDown", ambient_color_down);
	standardShader->setMat4("lightSpace", lightViewProjections[0], NUM_LIGHTS);
	standardShader->setVec3("u_lightDirection", lightDirections[0], NUM_LIGHTS);
	standardShader->setVec3("u_lightColor", lightColors[0], NUM_LIGHTS);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowMap[0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowMap[1]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glActiveTexture(GL_TEXTURE0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (auto renderable : renderables) {
		DrawRenderable(renderable);
	}
	
	// draw 2d objects
	DrawUI();
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data.data());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

void Renderer::updateInterpolationValue() {
	auto curr_time = std::chrono::high_resolution_clock::now();
	float elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>>(curr_time - interp_start).count();
	interp_value = elapsed_time / interp_duration;
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
	glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

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

	standardShader = new Shader("../assets/shaders/Standard.vsh", "../assets/shaders/Standard.psh");
	depthMapShader = new Shader("../assets/shaders/ShadowMap.vsh", "../assets/shaders/ShadowMap.psh");
	uiShader = new Shader("../assets/shaders/UI.vsh", "../assets/shaders/UI.psh");

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	standardShader->use();
	// shadow stuff
	standardShader->setFloat("u_shadowMapTexelSize", 1.0f / SHADOW_SIZE);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (int i = 0; i < NUM_LIGHTS; ++i) {
		glGenFramebuffers(1, &shadowMapFBO[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO[i]);
		glGenTextures(1, &shadowMap[i]);
		glBindTexture(GL_TEXTURE_2D, shadowMap[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap[i], 0);
		standardShader->setInt("u_shadowMapSampler[" + std::to_string(i) + "]", i + 1);
	}
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
	// binds UNIFORM_DEPTHMAP_SAMPLER to GL_TEXTURE3
	standardShader->setInt("u_depthMapSampler", 3);
	// binds SSAO kernel
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;
	generator.seed(2);
	int num_samples = 8;
	std::vector<glm::vec3> ssaoKernel;
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
	standardShader->setVec3("u_samples", ssaoKernel[0], num_samples);
	standardShader->setInt("u_numSamples", num_samples);

	// use GPU linear to SRGB conversion
	glEnable(GL_FRAMEBUFFER_SRGB);

	// use depth culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// use alpha for transperancy
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// use multisample anti-aliasing
	glEnable(GL_MULTISAMPLE);

	// set opengl to swap framebuffer every # screen refreshes
	glfwSwapInterval(1);

	glm::vec3 linear_ambient_color_up = glm::convertSRGBToLinear(ambient_color_up);

	PreloadAssetBuffers();
	InputManager::registerInputCallbacks(window);

	directionalLight[1].color = glm::vec3(0.7f, 0.8f, 0.9f);
	directionalLight[1].direction = glm::vec3(0, -1, 0);
	directionalLight[1].nearclip = -50;
	directionalLight[1].farclip = 50;

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
		updateInterpolationValue();
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

void Renderer::DrawUI() {
	UIManager::Root()->screenSize = glm::vec2(windowWidth, windowHeight);
	UIManager::Root()->Resize();
	glm::mat4 ui_viewProjection = glm::ortho(0.0f, (GLfloat)windowWidth, 0.0f, (GLfloat)windowHeight, -100.0f, 100.0f);
	uiShader->use();
	uiShader->setMat4("viewProjection", ui_viewProjection);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_DEPTH_BUFFER_BIT);
	transparentList.clear();

	TraverseUIComponent(UIManager::Root());

	glDepthFunc(GL_LEQUAL);
	transparentList.sort([](const UIComponent *f, const UIComponent *s) { return f->z < s->z; });
	for (UIComponent *t : transparentList) {
		DrawUIComponent(t);
	}
	glDepthFunc(GL_ALWAYS);
	UIComponent* black = UIManager::GetComponentById("BlackOverlay");
	if (black != nullptr && black->visible) {
		DrawUIComponent(black);
	}
	glDepthFunc(GL_LESS);
}
void Renderer::TraverseUIComponent(UIComponent *component) {
	if (component == nullptr || !component->visible || component->id == "BlackOverlay") {
		return;
	}

	if (component->IsTransparent()) {
		transparentList.push_back(component);
	} else {
		DrawUIComponent(component);
	}

	for (UIComponent *child : component->children) {
		TraverseUIComponent(child);
	}
}
void Renderer::InitUIComponent(UIComponent * renderable) {
	glDeleteBuffers(1, &renderable->model.positionLoc);
	glDeleteBuffers(1, &renderable->model.UVLoc);
	glDeleteBuffers(1, &renderable->model.normalLoc);
	glDeleteBuffers(1, &renderable->model.elementLoc);
	glGenBuffers(1, &renderable->model.positionLoc);
	glGenBuffers(1, &renderable->model.UVLoc);
	glGenBuffers(1, &renderable->model.normalLoc);
	glGenBuffers(1, &renderable->model.elementLoc);
	glBindBuffer(GL_ARRAY_BUFFER, renderable->model.positionLoc);
	glBufferData(GL_ARRAY_BUFFER, renderable->model.positions.size() * sizeof(glm::vec3), renderable->model.positions.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, renderable->model.UVLoc);
	glBufferData(GL_ARRAY_BUFFER, renderable->model.UVs.size() * sizeof(glm::vec2), renderable->model.UVs.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, renderable->model.normalLoc);
	glBufferData(GL_ARRAY_BUFFER, renderable->model.normals.size() * sizeof(glm::vec3), renderable->model.normals.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable->model.elementLoc);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderable->model.elements.size() * sizeof(GLuint), renderable->model.elements.data(), GL_STATIC_DRAW);
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
			directionalLight[0] = p->Param;
			break;
		}

		case RENDERER_SET_AMBIENT_UP: {
			TypeParam<glm::vec3> *p = dynamic_cast<TypeParam<glm::vec3> *>(params);
			ambient_color_up = glm::convertSRGBToLinear(p->Param);
			break;
		}

		case RENDERER_SET_FLOOR_COLOR: {
			TypeParam<glm::vec3> *p = dynamic_cast<TypeParam<glm::vec3> *>(params);
			floor_color = glm::convertSRGBToLinear(p->Param);
			break;
		}

		case FIXED_UPDATE_FINISHED: {
			renderables_mutex.lock();
			TypeParam<float> *p = dynamic_cast<TypeParam<float> *>(params);
			interp_duration = p->Param;
			interp_start = std::chrono::high_resolution_clock::now();
			for (auto &renderable : renderables) {
				if (renderable->interpolated) {
					renderable->renderPositionPrev = renderable->renderPositionCur;
					renderable->renderRotationPrev = renderable->renderRotationCur;
					renderable->renderScalePrev = renderable->renderScaleCur;
				}
				renderable->renderPositionCur = renderable->pos;
				renderable->renderRotationCur = renderable->rot;
				renderable->renderScaleCur = renderable->size;
			}
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
	EventManager::subscribe(RENDERER_SET_FLOOR_COLOR, this);
	EventManager::subscribe(FIXED_UPDATE_FINISHED, this);
}

Renderer::~Renderer() {
	renderThreadDone = true;
	renderThread.join();
	EventManager::unsubscribe(RENDERER_ADD_TO_RENDERABLES, this);
	EventManager::unsubscribe(RENDERER_SET_CAMERA, this);
	EventManager::unsubscribe(RENDERER_SET_DIRECTIONAL_LIGHT, this);
	EventManager::unsubscribe(RENDERER_SET_AMBIENT_UP, this);
	EventManager::unsubscribe(RENDERER_SET_FLOOR_COLOR, this);
	EventManager::unsubscribe(FIXED_UPDATE_FINISHED, this);
}