#pragma once
#include "GameObject.h"
class Hazard :
	public GameObject
{
private:
	float x;
	float y;
	float z;

public:
	Hazard();
	float getX();
	float getY();
	float getZ();

	void setX(float* a);
	void setY(float* a);
	void setZ(float* a);
	~Hazard();

};

