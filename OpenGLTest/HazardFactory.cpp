#include "HazardFactory.h"

glm::vec2 HazardFactory::floorSize = glm::vec2(32.0f);

int randInRange(int min, int max) {
	return min + (rand() % (max - min + 1));
}

Hazard* HazardFactory::buildCube(float size) {
	return buildPrism(glm::vec3(size));
}

Hazard* HazardFactory::buildPrism(glm::vec3 size) {
	int MAX_X = floorSize[0] - size.x;
	int MAX_Z = floorSize[1] - size.z;

	float X = randInRange(-MAX_X, MAX_X);
	float Y = randInRange(10, 20) + size.y;
	float Z = randInRange(-MAX_Z, MAX_Z);
	glm::vec3 pos = glm::vec3(X, Y, Z);
	float fallSpeed = 5.0f;

	Hazard* hazard = new Hazard(pos, size, fallSpeed);
	return hazard;
}