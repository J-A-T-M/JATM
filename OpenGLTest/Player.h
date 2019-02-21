#pragma once
#include "GameObject.h"

class Player : public GameObject {
	public:
		Player();
		~Player();

		float getRadius();
		void setRadius(float radius);
	protected:
		float _radius;
};

