#include "Hazard.h"

Hazard::Hazard(glm::vec3 spawnPosition, glm::vec3 size, float fall_speed) {
	fallSpeed = fall_speed;

	setLocalPosition(spawnPosition);
	setSize(size);
	addRenderable(Colour::PLATINUM, MODEL_CUBE_BEVEL, TEXTURE_NONE, 0.25f, 0.0f);
	renderable->interpolated_ = true;
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
	return (_localPosition.y <= _size.y);
}

ModelEnum Hazard::HazardModel() {
	int q = rand() % 2;
	switch (q){
	case 0:
		return MODEL_CUBE_BEVEL;
		break;
	case 1:
		return MODEL_CUBE;
		break;
	default:
		return MODEL_SPHERE;
		break;
	}

}