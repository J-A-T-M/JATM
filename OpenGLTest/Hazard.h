#pragma once
#include "GameObject.h"
class Hazard : public GameObject {
	public:
		Hazard();
		~Hazard();

		void update(float delta);
		bool grounded();

		float fallSpeed;
};

