#pragma once
#include "GameObject.h"
class Hazard : public GameObject {
	public:
		Hazard(glm::vec3 spawnPosition, glm::vec3 size, float fallSpeed);
		~Hazard();

		void update(float delta);
		bool grounded();
		ModelEnum HazardModel();

		float fallSpeed;
};

