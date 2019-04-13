#include "Quadtree.h"

Quadtree::Quadtree(int level, Rect * bounds) {
	objects = {};
	nodes = {};
	this->level = level;
	this->bounds = bounds;
}

Quadtree::~Quadtree() {
	Clear();
	delete bounds;
}

void Quadtree::Clear() {
	for (Rect* object : objects) {
		delete object;
	}
	objects.clear();

	for (Quadtree* node : nodes) {
		delete node;
	}
	nodes.clear();
}

void Quadtree::Insert(Rect * rect) {
	if (nodes.size() > 0 && nodes[0] != nullptr) {
		int index = GetIndex(rect);

		if (index != -1) {
			nodes[index]->Insert(rect);
			return;
		}
	}

	objects.push_back(rect);

	if (objects.size() > MAX_OBJECTS && level < MAX_LEVELS) {
		if (nodes.size() == 0 || nodes[0] == NULL)
			Split();

		size_t i = 0;
		while (i < objects.size()) {
			int index = GetIndex(objects[i]);
			if (index != -1) {
				nodes[index]->Insert(objects[i]);
				objects.erase(objects.begin() + i);
			} else
				i++;
		}
	}
}

std::vector<int>* Quadtree::Retrieve(std::vector<int>* result, Rect * rect) {
	// push back object IDs from this quadtree
	for (size_t i = 0; i < objects.size(); ++i) {
		result->push_back(objects[i]->id);
	}

	int index = GetIndex(rect);
	if (index == -1) { // push back object IDs from all children
		for (size_t i = 0; i < nodes.size(); ++i) {
			if (nodes[i] != NULL) {
				nodes[i]->Retrieve(result, rect);
			}
		}
	} else { // push back object IDs from child given by index
		if (nodes.size() != 0) {
			if (nodes[index] != NULL) {
				nodes[index]->Retrieve(result, rect);
			}
		}
	}

	return result;
}

void Quadtree::Split() {
	float subWidth = bounds->width / 2.0f;
	float subHeight = bounds->height / 2.0f;
	float x = bounds->x;
	float y = bounds->y;

	Quadtree* subQuad1 = new Quadtree(level + 1, new Rect(x, y + subHeight, subWidth, subHeight));
	Quadtree* subQuad2 = new Quadtree(level + 1, new Rect(x + subWidth, y + subHeight, subWidth, subHeight));
	Quadtree* subQuad3 = new Quadtree(level + 1, new Rect(x, y, subWidth, subHeight));
	Quadtree* subQuad4 = new Quadtree(level + 1, new Rect(x + subWidth, y, subWidth, subHeight));

	nodes.push_back(subQuad1);
	nodes.push_back(subQuad2);
	nodes.push_back(subQuad3);
	nodes.push_back(subQuad4);
}

int Quadtree::GetIndex(Rect * rect) {
	double horizontalMidpoint = bounds->x + bounds->width * 0.5;
	double verticalMidpoint = bounds->y + bounds->height * 0.5;

	bool inLeft = (rect->x < horizontalMidpoint);
	bool inRight = (rect->x + rect->width >= horizontalMidpoint);
	bool inTop = (rect->y < verticalMidpoint);
	bool inBottom = (rect->y + rect->height >= verticalMidpoint);

	if (inLeft == inRight || inTop == inBottom) {
		return -1;
	}

	return inRight + 2 * inBottom;
}