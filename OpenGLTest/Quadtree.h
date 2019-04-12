#pragma once
#include <vector>
#include "Rectangle.h"

class Quadtree {
	public:
		Quadtree(int level, Rectangle* bounds);
		~Quadtree();
		void Clear();
		void Insert(Rectangle* rect);
		std::vector<int>* Retrieve(std::vector<int>* result, Rectangle* rect);
	private:
		static const int MAX_OBJECTS = 1;
		static const int MAX_LEVELS = 8;

		int level;
		Rectangle* bounds;
		std::vector<Rectangle*> objects;
		std::vector<Quadtree*> nodes;

		void Split();
		int GetIndex(Rectangle* rect);
};