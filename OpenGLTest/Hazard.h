#pragma once
#include "GameObject.h"
class Hazard : public GameObject {
	public:
		Hazard(glm::vec3 spawnPosition, float fallSpeed);
		~Hazard();

		void update(float delta);
		bool grounded();

		float fallSpeed;
};

