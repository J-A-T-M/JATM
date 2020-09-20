#include "Renderable.h"

Renderable::Renderable(glm::vec3 color, ModelEnum model, TextureEnum texture, float roughness, float metallic, TextureEnum normalMap, TextureEnum dispMap) {
	this->color = color;
	this->model = model;
	this->texture = texture;
	this->normalMap = normalMap;
	this->dispMap = dispMap;
	this->roughnessMap = TEXTURE_NONE;
	this->roughness = roughness;
	this->metallic = metallic;

	fullBright = false;

	m = {};
	parent = nullptr;
	currPos = glm::vec3(0.0f);
	currRot = glm::vec3(0.0f);
	currSize = glm::vec3(1.0f);
	prevPos = glm::vec3(0.0f);
	prevRot = glm::vec3(0.0f);
	prevScale = glm::vec3(1.0f);
}
