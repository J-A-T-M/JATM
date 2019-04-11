#pragma once
#include <vector>
#include "Rectangle.h"

class Quadtree
{
private:
	const int MAX_OBJECTS = 1;
	const int MAX_LEVELS = 8;

	int level;
	std::vector<Rectangle*> objects = {};
	Rectangle* bounds;
	std::vector<Quadtree*> nodes = {};

	void Split()
	{
		float subWidth = this->bounds->width / 2.0;
		float subHeight = this->bounds->height / 2.0;
		int x = this->bounds->x;
		int y = this->bounds->y;

		Quadtree* subQuad1 = new Quadtree(this->level + 1, new Rectangle(x + subWidth, y, subWidth, subHeight));
		Quadtree* subQuad2 = new Quadtree(this->level + 1, new Rectangle(x, y, subWidth, subHeight));
		Quadtree* subQuad3 = new Quadtree(this->level + 1, new Rectangle(x, y + subHeight, subWidth, subHeight));
		Quadtree* subQuad4 = new Quadtree(this->level + 1, new Rectangle(x + subWidth, y + subHeight, subWidth, subHeight));

		this->nodes.push_back(subQuad1);
		this->nodes.push_back(subQuad2);
		this->nodes.push_back(subQuad3);
		this->nodes.push_back(subQuad4);
	}

	int GetIndex(Rectangle* rect)
	{
		int index = -1;

		double verticalMidpoint = bounds->x + (bounds->width / 2.0);
		double horizontalMidpoint = bounds->y + (bounds->height / 2.0);

		bool inLeft = (rect->x < verticalMidpoint && rect->x + rect->width < verticalMidpoint);
		bool inRight = (rect->x >= verticalMidpoint && rect->x + rect->width >= verticalMidpoint);
		bool inTop = (rect->y < horizontalMidpoint && rect->y + rect->height < horizontalMidpoint);
		bool inBottom = (rect->y >= horizontalMidpoint && rect->y + rect->height >= horizontalMidpoint);

		if (inLeft) 
		{
			if (inTop)
				index = 0;
			else if (inBottom)
				index = 3;
		}
		else if (inRight)
		{
			if (inTop)
				index = 1;
			else if (inBottom)
				index = 2;
		}

		return index;
	}

public:
	Quadtree(int level, Rectangle* bounds)
	{
		this->level = level;
		this->bounds = bounds;
	}

	~Quadtree() {
		Clear();
		delete bounds;
	}

	void Clear() {
		for (Rectangle* object : objects) {
			delete object;
		}
		objects.clear();

		for (Quadtree* node : nodes) {
			delete node;
		}
		nodes.clear();
	}

	void Insert(Rectangle* rect)
	{
		if (this->nodes.size() > 0 && this->nodes[0] != NULL)
		{
			int index = this->GetIndex(rect);

			if (index != -1)
			{
				this->nodes[index]->Insert(rect);
				return;
			}
		}

		this->objects.push_back(rect);

		if (this->objects.size() > this->MAX_OBJECTS && this->level < this->MAX_LEVELS)
		{
			if (this->nodes.size() == 0 || this->nodes[0] == NULL)
				this->Split();

			int i = 0;
			while (i < this->objects.size())
			{
				int index = this->GetIndex(this->objects[i]);
				if (index != -1)
				{
					this->nodes[index]->Insert(this->objects[i]);
					this->objects.erase(this->objects.begin() + i);
				}
				else
					i++;
			}
		}
	}

	std::vector<int>* Retrieve(std::vector<int>* result, Rectangle* rect)
	{	
		// push back object IDs from this quadtree
		for (int i = 0; i < objects.size(); ++i) {
			result->push_back(objects[i]->id);
		}

		int index = GetIndex(rect);
		if (index == -1) { // push back object IDs from all children
			for (int i = 0; i < nodes.size(); ++i) {
				if (nodes[i] != NULL) {
					nodes[i]->Retrieve(result, rect);
				}
			}
		} else { // push back object IDs from child given by index
			if (index < nodes.size()) {
				if (nodes[index] != NULL) {
					nodes[index]->Retrieve(result, rect);
				}
			}
		}

		return result;
	}
};