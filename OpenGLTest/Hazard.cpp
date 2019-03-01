#include "Hazard.h"

Hazard::Hazard(glm::vec3 spawnPosition, float fall_speed) {
	fallSpeed = fall_speed;

	setLocalPosition(spawnPosition);
	setLocalScale(2.5f);
	addRenderable();
	renderable->roughness = 0.25;
	renderable->color = glm::vec4(0.75, 0.75, 0.75, 1.0);
	renderable->model = MODEL_CUBE_BEVEL;
	renderable->interpolated = true;

	clearRenderablePreviousTransforms();
}

Hazard::~Hazard() {
}

void Hazard::update(float delta) {
	glm::vec3 pos = _localPosition;
	pos.y -= fallSpeed * delta;
	if (pos.y < _localScale) {
		pos.y = _localScale;
	}
	setLocalPosition(pos);
}

bool Hazard::grounded() {
	return (_localPosition.y <= _localScale);
}
