#include "Player.h"
#include <algorithm>

Player::Player() {
	setLocalScale(2.0f);
	_radius = 2.0f;
	_force = glm::vec3(0);
	_velocity = glm::vec3(0);
	_bounceUp = false;
	_health = STARTING_HEALTH;
	_invulnFrames = 0;
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

float Player::getForceY() {
	return _force.y;
}

glm::vec2 Player::getForceXZ() {
	return glm::vec2(_force.x, _force.z);
}

glm::vec3 Player::getForce()
{
	return _force;
}

void Player::setForceY(float y) {
	_force.y = y;
}

void Player::setForceXZ(glm::vec2 xz) {
	_force.x = xz.x;
	_force.z = xz.y;
}

void Player::setForce(glm::vec3 force)
{
	_force = force;
}

float Player::getVelocityY() {
	return _velocity.y;
}

glm::vec2 Player::getVelocityXZ() {
	return glm::vec2(_velocity.x, _velocity.z);
}
 
glm::vec3 Player::getVelocity()
{
	return _velocity;
}

void Player::setVelocityY(float y) {
	_velocity.y = y;
}

void Player::setVelocityXZ(glm::vec2 xz) {
	_velocity.x = xz.x;
	_velocity.z = xz.y;
}

void Player::setVelocity(glm::vec3 velocity)
{
	_velocity = velocity;
}

bool Player::getBounceUp()
{
	return _bounceUp;
}

void Player::setBounceUp(bool flag)
{
	_bounceUp = flag;
}

int Player::getHealth() {
	return _health;
}

void Player::damageHealth(int damage) {
	if (_invulnFrames == 0) {
		_invulnFrames = MAX_INVULN_FRAMES;
		Player::setHealth(_health - damage);
	}
}

void Player::setHealth(int health) {
	if (health >= 0) {
		_health = health;
	} else {
		_health = 0;
	}
}

void Player::update() {
	renderable->fullBright = _invulnFrames % 2;

	if (_invulnFrames > 0) {
		--_invulnFrames;
	}
}
