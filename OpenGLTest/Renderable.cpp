#include "Renderable.h"

Renderable::Renderable(glm::vec3 color, ModelEnum model, TextureEnum texture, float roughness, float metallic) {
	this->color_ = color;
	this->model_ = model;
	this->texture_ = texture;
	this->roughness_ = roughness;
	this->metallic_ = metallic;

	interpolated_ = false;
	fullBright_ = false;

	pos = glm::vec3(0.0f);
	rot = glm::vec3(0.0f);
	size = glm::vec3(1.0f);

	renderPositionCur = glm::vec3(0.0f);
	renderRotationCur = glm::vec3(0.0f);
	renderScaleCur = glm::vec3(1.0f);
	renderPositionPrev = glm::vec3(0.0f);
	renderRotationPrev = glm::vec3(0.0f);
	renderScalePrev = glm::vec3(1.0f);
}
