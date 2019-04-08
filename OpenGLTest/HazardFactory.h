#include "Hazard.h"

class HazardFactory
{
public:
	
	HazardFactory();
	~HazardFactory();
};

class Cube : public HazardFactory {

};

class Prisim : public HazardFactory {

};