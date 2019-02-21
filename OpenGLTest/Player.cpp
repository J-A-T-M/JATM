#include "Player.h"

Player::Player() {
	setLocalScale(2.0f);
	_radius = 2.0f;
	addRenderable();
	renderable->roughness = 0.4f;
	renderable->model = MODEL_SUZANNE;
	renderable->interpolated = true;
}

Player::~Player() {}

float Player::getRadius() {
	return _radius;
}

void Player::setRadius(float radius) {
	_radius = radius;
}
