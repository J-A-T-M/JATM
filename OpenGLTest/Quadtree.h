#pragma once
#include <vector>
#include "Rect.h"

class Quadtree {
	public:
		Quadtree(int level, Rect bounds);
		~Quadtree();
		void Clear();
		void Insert(Rect* rect);
		std::vector<int>* Retrieve(std::vector<int>* result, Rect* rect);
	private:
		static const int MAX_OBJECTS = 1;
		static const int MAX_LEVELS = 8;

		int level;
		Rect bounds;
		std::vector<Rect*> objects;
		std::vector<Quadtree*> nodes;

		void Split();
		int GetIndex(Rect* rect);
};