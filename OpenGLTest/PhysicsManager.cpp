#include "PhysicsManager.h"

#include <glm/glm.hpp>

void PhysicsManager::Update(std::vector<Player*> &players, const float delta)
{
	// std::cout << "[PHY] Updated" << std::endl;

	// move players
	// fixme: server can't process fast enough if a player presses the keys for too long
	const float PLAYER_SPEED = 30.0;
	const float FORCE_INCREMENT = 0.1;
	for (int i = 0; i < players.size(); ++i)
	{
		glm::vec3 pos = players[i]->getLocalPosition();

		glm::vec3 force = players[i]->getForce();
		glm::vec3 thrust = players[i]->getThrust();

		if (thrust.x != 0)
			force.x += thrust.x * FORCE_INCREMENT * delta;
		else if (force.x < 0)
			force.x += FORCE_INCREMENT * delta;
		else if (force.x > 0)
			force.x -= FORCE_INCREMENT * delta;
		if (force.x < -1) force.x = -1;
		else if (force.x > 1) force.x = 1;
		else if (thrust.x == 0 && (force.x > 0 && force.x < FORCE_INCREMENT || force.x < 0 && force.x > -FORCE_INCREMENT)) force.x = 0;

		if (thrust.z != 0)
			force.z += thrust.z * FORCE_INCREMENT* delta;
		else if (force.z < 0)
			force.z += FORCE_INCREMENT * delta;
		else if (force.z > 0)
			force.z -= FORCE_INCREMENT * delta;
		if (force.z < -1) force.z = -1;
		else if (force.z > 1) force.z = 1;
		else if (thrust.z == 0 && (force.z > 0 && force.z < FORCE_INCREMENT || force.z < 0 && force.z > -FORCE_INCREMENT)) force.z = 0;

		players[i]->setForce(force);

		glm::vec3 movement = glm::vec3(force.x, 0.0f, force.z);
		if (movement != glm::vec3(0))
		{
			movement = normalize(movement);
		}
		pos += movement * PLAYER_SPEED * delta;
		players[i]->setLocalPosition(pos);
	}

	// collision detection
	for (int i = 0; i < players.size(); ++i)
	{
		for (int j = 0; j < players.size(); ++j)
		{
			if (i >= j)
				continue;

			glm::vec3 posA = players[i]->getLocalPosition();
			glm::vec3 posB = players[j]->getLocalPosition();

			float radiusA = players[i]->getRadius();
			float radiusB = players[j]->getRadius();

			float minDistanceAllowed = radiusA + radiusB;

			float x2x1 = posB.x - posA.x;
			float z2z1 = posB.z - posA.z;
			float distanceBetween = sqrtf(x2x1 * x2x1 + z2z1 * z2z1);

			if (distanceBetween <= minDistanceAllowed)
			{
				// std::cout << "Collision detected: Player" << i << " and Player" << j << std::endl;
				// todo: collision correction properly..
				glm::vec3 forceA = players[i]->getForce();
				glm::vec3 forceB = players[j]->getForce();

				players[i]->setForce(-forceA * glm::vec3(0.5));
				players[j]->setForce(-forceB * glm::vec3(0.5));
			}
		}
	}
}
