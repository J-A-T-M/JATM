#include "Hazard.h"

Hazard::Hazard() {
	setLocalScale(2.5f);
	addRenderable();
	renderable->roughness = 0.25;
	renderable->color = glm::vec4(0.75, 0.75, 0.75, 1.0);
	renderable->model = HazardModel();
	renderable->interpolated = true;
}

Hazard::~Hazard() {
}

void Hazard::update(float delta) {
	glm::vec3 pos = getLocalPosition();
	pos.y -= fallSpeed * delta;
	if (pos.y < getLocalScale()) {
		pos.y = getLocalScale();
	}
	setLocalPosition(pos);
}

bool Hazard::grounded() {
	return (getLocalPosition().y == getLocalScale());
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