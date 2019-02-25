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

		glm::vec3 getThrust();
		void setThrust(glm::vec3 thrust);
	protected:
		float _radius;
		glm::vec3 _force;
		glm::vec3 _thrust;
};

