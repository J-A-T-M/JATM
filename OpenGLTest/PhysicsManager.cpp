#include "PhysicsManager.h"

#include <glm/glm.hpp>

void PhysicsManager::Update(std::vector<Player*> &players, const float delta)
{
	const float PLAYER_ACCELERATION = 90.0;
	const float PLAYER_MAX_SPEED = 30.0;

	const float PLAYER_BOUNCE_MAX = 1.0;
	const float PLAYER_BOUNCE_INCREMENT = 10.0;
	const float PLAYER_BASE_HEIGHT = 2.0;

	for (int i = 0; i < players.size(); ++i) {
		glm::vec3 pos = players[i]->getLocalPosition();
		glm::vec3 force = players[i]->getForce();
		glm::vec3 velocity = players[i]->getVelocity();

		if (force.x != 0)
			velocity.x += force.x * PLAYER_ACCELERATION * delta;
		else
			velocity.x = 0;
		if (velocity.x < -PLAYER_MAX_SPEED) velocity.x = -PLAYER_MAX_SPEED;
		else if (velocity.x > PLAYER_MAX_SPEED) velocity.x = PLAYER_MAX_SPEED;

		if (force.z != 0)
			velocity.z += force.z * PLAYER_ACCELERATION * delta;
		else
			velocity.z = 0;
		if (velocity.z < -PLAYER_MAX_SPEED) velocity.z = -PLAYER_MAX_SPEED;
		else if (velocity.z > PLAYER_MAX_SPEED) velocity.z = PLAYER_MAX_SPEED;

		if (glm::length(velocity) > PLAYER_MAX_SPEED) {
			velocity = normalize(velocity) * PLAYER_MAX_SPEED;
		}
		players[i]->setVelocity(velocity);

		glm::vec3 movement = glm::vec3(velocity.x, 0.0f, velocity.z) * delta;
		pos += movement;

		float jumpHeight = pos.y - PLAYER_BASE_HEIGHT;
		if (force.x != 0 || force.z != 0)
		{
			// rotation
			float angle = atan2(force.x, force.z);
			glm::vec3 rotation = glm::vec3(0, angle, 0);
			players[i]->setLocalRotation(rotation);

			// bounce
			if (jumpHeight < PLAYER_BOUNCE_MAX && players[i]->getBounceUp())
			{
				jumpHeight += PLAYER_BOUNCE_INCREMENT * delta;
				if (jumpHeight > PLAYER_BOUNCE_MAX) jumpHeight = PLAYER_BOUNCE_MAX;
			}
		}

		// bounce fall
		if (jumpHeight > 0 && !players[i]->getBounceUp())
		{
			jumpHeight -= PLAYER_BOUNCE_INCREMENT * delta;
			if (jumpHeight < 0) jumpHeight = 0;
		}

		pos.y = PLAYER_BASE_HEIGHT + jumpHeight;

		if (jumpHeight == 0) players[i]->setBounceUp(true);
		if (jumpHeight == PLAYER_BOUNCE_MAX || (force.x == 0 && force.z == 0)) players[i]->setBounceUp(false);

		players[i]->setLocalPosition(pos);
	}

	// collision detection
	for (int i = 0; i < players.size(); ++i) {
		for (int j = i + 1; j < players.size(); ++j) {
			glm::vec3 posA = players[i]->getLocalPosition();
			glm::vec3 posB = players[j]->getLocalPosition();
			glm::vec3 velocityA = players[i]->getVelocity();
			glm::vec3 velocityB = players[j]->getVelocity();
			float radiusA = players[i]->getRadius();
			float radiusB = players[j]->getRadius();

			glm::vec3 normal = posA - posB;

			// todo: a positionXZ getter
			normal.y = 0;

			float dist = glm::length(normal);
			normal /= dist;
			dist -= radiusA + radiusB;

			if (dist < 0.0f) {
				// if we want to make it easier to push other players we can
				// multiply the normal by abs of itself to make it more axis
				// alligned
				/*
				normal *= abs(normal);
				normal = normalize(normal);
				*/
				glm::vec3 avg_pos = (posA + posB) * 0.5f;
				float avg_radius = (radiusA + radiusB) * 0.5f;
				posA = avg_pos + normal * avg_radius;
				posB = avg_pos - normal * avg_radius;
				players[i]->setLocalPosition(posA);
				players[j]->setLocalPosition(posB);
			}
		}
	}
}
