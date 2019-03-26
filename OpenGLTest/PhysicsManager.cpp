#include "PhysicsManager.h"

#include <glm/glm.hpp>
Quadtree* PhysicsManager::quad = new Quadtree(0, new Rectangle(0, 0, 64, 64));

bool intersects(glm::vec2 distance, float radius, float size) {
	if (distance.x > (size + radius)) { return false; }
	if (distance.y > (size + radius)) { return false; }

	if (distance.x <= size) { return true; }
	if (distance.y <= size) { return true; }

	glm::vec2 cornerDistance = distance - size;
	float cornerDistance_sq = glm::dot(cornerDistance * cornerDistance, glm::vec2(1.0));
	return (cornerDistance_sq <= (radius * radius));
}

void PhysicsManager::Update(std::vector<Player*> &players, std::vector<Hazard*> &hazards, const float delta)
{
	const float PLAYER_ACCELERATION = 90.0;
	const float PLAYER_MAX_SPEED = 30.0;

	const float PLAYER_BOUNCE_MAX = 1.0;
	const float PLAYER_BOUNCE_INCREMENT = 10.0;
	const float PLAYER_BASE_HEIGHT = 2.0;

	for (int i = 0; i < players.size(); ++i) {
		glm::vec3 rot = players[i]->getLocalRotation();
		glm::vec3 pos = players[i]->getLocalPosition();
		glm::vec3 force = players[i]->getForce();
		glm::vec3 velocity = players[i]->getVelocity();

		if (players[i]->getHealth() == 0) {
			force = glm::vec3(0);
			rot.x = glm::half_pi<float>();
		}

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

		float baseHeight = PLAYER_BASE_HEIGHT * ((float)players[i]->getHealth() / (float)Player::STARTING_HEALTH);
		float jumpHeight = glm::max(pos.y - baseHeight, 0.0f);
		if (force.x != 0 || force.z != 0)
		{
			// rotation
			float angle = atan2(force.x, force.z);
			rot.y = angle;

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

		pos.y = baseHeight + jumpHeight;

		if (jumpHeight <= 0) players[i]->setBounceUp(true);
		if (jumpHeight >= PLAYER_BOUNCE_MAX || (force.x == 0 && force.z == 0)) players[i]->setBounceUp(false);

		pos.x = glm::clamp(pos.x, -30.0f, 30.0f);
		pos.z = glm::clamp(pos.z, -30.0f, 30.0f);

		players[i]->setLocalRotation(rot);
		players[i]->setLocalPosition(pos);
	}

	// insert players and hazards into quadtree
	PhysicsManager::quad->Clear();
	for (int i = 0; i < players.size(); ++i)
	{
		float radius = players[i]->getRadius();
		glm::vec2 playerPosition = players[i]->getLocalPositionXZ();
		Rectangle* pRect = new Rectangle(i, playerPosition.x - radius, playerPosition.y - radius, radius * 2, radius * 2);
		PhysicsManager::quad->Insert(pRect);
	}
	for (int i = 0; i < hazards.size(); ++i)
	{
		glm::vec2 hazardPosition = hazards[i]->getLocalPositionXZ();
		float size = hazards[i]->getLocalScale();
		Rectangle* hRect = new Rectangle(i + 100, hazardPosition.x - size / 2.0, hazardPosition.y - size / 2.0, size, size);
		PhysicsManager::quad->Insert(hRect);
	}

	// fetch from quadtree and do collision detection
	std::vector<int> closeBy;
	for (int i = 0; i < players.size(); ++i)
	{
		closeBy.clear();

		float radius = players[i]->getRadius();
		glm::vec2 playerPosition = players[i]->getLocalPositionXZ();
		Rectangle* pRect = new Rectangle(i, playerPosition.x - radius, playerPosition.y - radius, radius * 2, radius * 2);

		PhysicsManager::quad->Retrieve(&closeBy, pRect);

		for (int j = 0; j < closeBy.size(); ++j)
		{
			int goId = closeBy[j];

			if (goId >= 100) // hazzards
			{
				int hId = goId - 100;

				glm::vec2 playerPosition = players[i]->getLocalPositionXZ();
				glm::vec2 hazardPosition = hazards[hId]->getLocalPositionXZ();
				glm::vec2 distance = glm::abs(playerPosition - hazardPosition);
				float radius = players[i]->getRadius();
				float size = hazards[hId]->getLocalScale();
				float playerHeight = players[i]->getLocalPositionY();
				float hazardHeight = hazards[hId]->getLocalPositionY();
				if (hazardHeight <= playerHeight + size + radius) {
					if (intersects(distance, radius, size)) {
						players[i]->damageHealth(25);
					}
				}
			}
			else if (i != goId)
			{
				// player vs player
				glm::vec2 posA = players[i]->getLocalPositionXZ();
				glm::vec2 posB = players[goId]->getLocalPositionXZ();
				glm::vec2 velocityA = players[i]->getVelocityXZ();
				glm::vec2 velocityB = players[goId]->getVelocityXZ();
				float radiusA = players[i]->getRadius();
				float radiusB = players[goId]->getRadius();

				glm::vec2 normal = posA - posB;

				float dist = glm::length(normal);
				normal /= dist;
				dist -= radiusA + radiusB;

				if (dist < 0.0f) {
					glm::vec2 avg_pos = (posA + posB) * 0.5f;
					float avg_radius = (radiusA + radiusB) * 0.5f;
					posA = avg_pos + normal * avg_radius;
					posB = avg_pos - normal * avg_radius;
					players[i]->setLocalPositionXZ(posA);
					players[goId]->setLocalPositionXZ(posB);

					if (normal.x == -1) {
						players[goId]->setStun(true);
						players[goId]->setBounceUp(false);
						players[goId]->setForce(glm::vec3(0.0, 0.0, 0.0));
					}
				}
			}
		}
	}
}
