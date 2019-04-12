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
	static void Update(std::vector<std::shared_ptr<Player>> &players, std::vector<std::shared_ptr<Hazard>> &hazards, const float delta);
};