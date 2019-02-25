#include "Player.h"

Player::Player() {
	setLocalScale(2.0f);
	_radius = 2.0f;
	_force = glm::vec3(0.0f, 0.0f, 0.0f);
	_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
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

glm::vec3 Player::getForce()
{
	return _force;
}

void Player::setForce(glm::vec3 force)
{
	_force = force;
}

glm::vec3 Player::getVelocity()
{
	return _velocity;
}

void Player::setVelocity(glm::vec3 velocity)
{
	_velocity = velocity;
}
