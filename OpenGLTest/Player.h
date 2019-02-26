#pragma once
#include "GameObject.h"
#include <glm/glm.hpp>

class Player : public GameObject {
	public:
		Player();
		~Player();

		float getRadius();
		void setRadius(float radius);

		glm::vec3 getForce();
		void setForce(glm::vec3 force);

		glm::vec3 getVelocity();
		void setVelocity(glm::vec3 velocity);

		bool getBounceUp();
		void setBounceUp(bool flag);
	protected:
		float _radius;
		glm::vec3 _force;
		glm::vec3 _velocity;
		bool _bounceUp;
};

