#pragma once
#include <iostream>
#include <vector>

#include "Player.h"
#include "Hazard.h"
#include "Quadtree.h"

class PhysicsManager
{
private:
	static Quadtree* quad;
public:
	static void Update(std::vector<Player*> &players, std::vector<Hazard*> &hazards, const float delta);
};