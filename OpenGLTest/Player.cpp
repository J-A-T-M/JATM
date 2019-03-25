#include "Player.h"
#include <algorithm>

const float BASE_ROUGHNESS = 0.4f; 
const float BASE_METALLIC = 1.0f;
#define STUN_COLOR Colour::TITANIUM

Player::Player(glm::vec2 xz, glm::vec3 color, float radius) : BASE_COLOR(color) {
	setLocalPositionXZ(xz);
	setLocalPositionY(radius);
	setLocalScale(radius);
	
	_invulnFrames = 0;
	_stunFrames = 0;
	_radius = radius;
	_force = glm::vec3(0);
	_velocity = glm::vec3(0);
	_bounceUp = false;
	_health = STARTING_HEALTH;

	addRenderable();
	renderable->color = BASE_COLOR;
	renderable->roughness = BASE_ROUGHNESS;
	renderable->metallic = BASE_METALLIC;
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
	if (_stunFrames > 0) {
		return 0.0f;
	}
	return _force.y;
}

glm::vec2 Player::getForceXZ() {
	if (_stunFrames > 0) {
		return glm::vec2(0.0f);
	}
	return glm::vec2(_force.x, _force.z);
}

glm::vec3 Player::getForce(){
	if (_stunFrames > 0) {
		return glm::vec3(0.0f);
	}
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
	if (_invulnFrames == 0 && _health != 0) {
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

void Player::setStun() {
	_stunFrames = MAX_STUN_FRAMES;
}
bool Player::getStun() {
	return _stunFrames > 0;
}


void Player::update() {
	renderable->fullBright = _invulnFrames % 2;

	float mixFactor = _stunFrames / (float)MAX_STUN_FRAMES;
	renderable->metallic = (_stunFrames == 0) ? BASE_METALLIC : 0.0f;
	renderable->roughness = (_stunFrames == 0) ? BASE_ROUGHNESS : 1.0f;
	renderable->color = glm::mix(BASE_COLOR, STUN_COLOR, mixFactor);

	if (_invulnFrames > 0) {
		--_invulnFrames;
	}
	if (_stunFrames > 0) {
		--_stunFrames;
	}
}