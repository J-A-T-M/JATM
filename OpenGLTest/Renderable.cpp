#include "Renderable.h"

Renderable::Renderable(glm::vec3 color, ModelEnum model, TextureEnum texture, float roughness, float metallic) {
	color_ = color;
	model_ = model;
	texture_ = texture;
	roughness_ = roughness;
	metallic_ = metallic;

	interpolated_ = false;
	fullBright_ = false;

	renderPositionCur = glm::vec3(0.0f);
	renderRotationCur = glm::vec3(0.0f);
	renderPositionPrev = glm::vec3(0.0f);
	renderRotationPrev = glm::vec3(0.0f);
}
