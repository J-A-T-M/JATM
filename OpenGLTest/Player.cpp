#include "Player.h"

Player::Player() {
	addRenderable();
	renderable->roughness = 0.4f;
	renderable->model = MODEL_SUZANNE;
	renderable->interpolated = true;
	renderable->scale = 2.0f;
	_radius = 2.0f;
}

Player::~Player() {}

float Player::getRadius() {
	return _radius;
}

void Player::setRadius(float radius) {
	_radius = radius;
}
