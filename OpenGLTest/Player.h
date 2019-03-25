#pragma once
#include "GameObject.h"
#include <glm/glm.hpp>

class Player : public GameObject {
	public:
		Player();
		~Player();

		float getRadius();
		void setRadius(float radius);

		float getForceY();
		glm::vec2 getForceXZ();
		glm::vec3 getForce();
		void setForceY(float y);
		void setForceXZ(glm::vec2 xz);
		void setForce(glm::vec3 force);

		float getVelocityY();
		glm::vec2 getVelocityXZ();
		glm::vec3 getVelocity();
		void setVelocityY(float y);
		void setVelocityXZ(glm::vec2 xz);
		void setVelocity(glm::vec3 velocity);

		bool getBounceUp();
		void setBounceUp(bool flag);

		void setStun();
		bool getStun();

		int getHealth();
		void damageHealth(int damage);
		void setHealth(int health);
		void update();

		const static int STARTING_HEALTH = 100;
		const static int MAX_INVULN_FRAMES = 10;
		const static int MAX_STUN_FRAMES = 20;
	protected:
		int _invulnFrames;
		int _stunFrames;

		float _radius;
		glm::vec3 _force;
		glm::vec3 _velocity;
		bool _bounceUp;
		int _health;
};

