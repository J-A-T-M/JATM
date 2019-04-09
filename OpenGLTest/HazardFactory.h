#include "Hazard.h"

class HazardFactory
{
public:
	static Hazard* buildCube(float size);
	static Hazard* buildPrism(glm::vec3 size);
	static glm::vec2 floorSize;
private:


};