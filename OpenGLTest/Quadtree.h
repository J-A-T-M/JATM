#pragma once
#include <vector>
#include "GameObject.h"
#include "Rectangle.h"

class Quadtree
{
private:
	const int MAX_OBJECTS = 3;
	const int MAX_LEVELS = 255;

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

		this->nodes.clear();

		this->nodes.push_back(subQuad1);
		this->nodes.push_back(subQuad2);
		this->nodes.push_back(subQuad3);
		this->nodes.push_back(subQuad4);
	}

	int GetIndex(Rectangle* rect)
	{
		int index = -1;
		double verticalMidpoint = this->bounds->x + (this->bounds->width / 2.0);
		double horizontalMidpoint = this->bounds->y + (this->bounds->height / 2.0);

		bool topQuadrant = (rect->y < horizontalMidpoint && rect->y + rect->height < horizontalMidpoint);
		bool bottomQuadrant = (rect->y > horizontalMidpoint);

		if (rect->x < verticalMidpoint && rect->x + rect->width < verticalMidpoint)
		{
			if (topQuadrant)
				index = 0;
			else
				index = 3;
		}
		else
		{
			if (topQuadrant)
				index = 1;
			else
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

	void Clear()
	{
		this->objects.clear();
		for (int i = 0; i < this->nodes.size(); ++i)
		{
			if (this->nodes.size() > 0 && this->nodes[i] != NULL)
			{
				this->nodes[i]->Clear();
				this->nodes[i] = NULL;
			}
		}
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
		int index = this->GetIndex(rect);
		if (index != -1 && this->nodes.size() > 0 && this->nodes[0] != NULL)
			this->nodes[index]->Retrieve(result, rect);

		for (int i = 0; i < this->objects.size(); ++i)
			result->push_back(this->objects[i]->id);
		return result;
	}
};