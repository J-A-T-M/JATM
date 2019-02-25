#include "Hazard.h"



Hazard::Hazard()
{
}


Hazard::~Hazard()
{
}

void Hazard::setX(float a) {
	Hazard::x = a;
}

void Hazard::setY(float a) {
	Hazard::y = a;
}

void Hazard::setZ(float a) {
	Hazard::z = a;
}

float getX() {
	return Hazard::x;
}

float getY() {
	return Hazard::y;
}

float getZ() {
	return Hazard::z;
}