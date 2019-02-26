#pragma once
#include <iostream>
#include <vector>

#include "Player.h"

class PhysicsManager
{
public:
	static void Update(std::vector<Player*> &players, const float delta);
};