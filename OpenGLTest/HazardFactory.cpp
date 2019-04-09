#include "HazardFactory.h"

HazardFactory::HazardFactory()
{
}

HazardFactory::~HazardFactory()
{
}

Hazard* HazardFactory::buildCube(float size) {
	glm::vec3 pos = glm::vec3(rand() % 58 - 29, 10 + rand() % 10, rand() % 58 - 29);
	float fallSpeed = 5.0f;
	Hazard* hazard = new Hazard(pos, fallSpeed);
	return hazard;
}

Hazard* HazardFactory::buildPrism(glm::vec3 size) {
	float Xscale = size.x;
	float Zscale = size.z;
	int A = 64, B = 32;
	int xBuffer = 64 - (Xscale * 6);
	int zBuffer = 64 - (Zscale * 6);

	float X = rand() % xBuffer - (xBuffer/2);
	float Z = rand() % zBuffer - (zBuffer/2);
	glm::vec3 pos = glm::vec3(X, 15, Z);
	float fallSpeed = 5.0f;
	Hazard* hazard = new Hazard(pos, fallSpeed);
	hazard->setNonLinearScale(glm::vec3(Xscale, 1.0, Zscale));
	return hazard;
}