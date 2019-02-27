#pragma once
#include <iostream>
#include <vector>

#include "Player.h"
#include "Hazard.h"

class PhysicsManager
{
public:
	static void Update(std::vector<Player*> &players, std::vector<Hazard*> &hazards, const float delta);
};