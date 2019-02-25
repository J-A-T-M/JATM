#include "PhysicsManager.h"

#include <glm/glm.hpp>

void PhysicsManager::Update(std::vector<Player*> &players, const float delta)
{
	const float PLAYER_SPEED = 30.0;
	for (int i = 0; i < players.size(); ++i) {
		glm::vec3 pos = players[i]->getLocalPosition();
		glm::vec3 thrust = players[i]->getForce();

		glm::vec3 movement = glm::vec3(thrust.x, 0.0f, thrust.z);
		if (movement != glm::vec3(0)) {
			movement = normalize(movement);
		}
		pos += movement * PLAYER_SPEED * delta;
		players[i]->setLocalPosition(pos);

		if (thrust.x != 0 || thrust.z != 0)
		{
			float angle = atan2(thrust.x, thrust.z);
			glm::vec3 rotation = glm::vec3(0, angle, 0);
			players[i]->setLocalRotation(rotation);
		}
	}

	// collision detection
	for (int i = 0; i < players.size(); ++i) {
		for (int j = i + 1; j < players.size(); ++j) {
			glm::vec3 posA = players[i]->getLocalPosition();
			glm::vec3 posB = players[j]->getLocalPosition();
			glm::vec3 thrustA = players[i]->getForce();
			glm::vec3 thrustB = players[j]->getForce();
			float radiusA = players[i]->getRadius();
			float radiusB = players[j]->getRadius();

			glm::vec3 normal = posA - posB;
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
