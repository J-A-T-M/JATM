#include "Hazard.h"

class HazardFactory
{
public:

	HazardFactory();
	~HazardFactory();
	static Hazard* buildCube(float size);
	static Hazard* buildPrism(glm::vec3 size);

private:


};