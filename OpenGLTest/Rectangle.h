#pragma once

class Rectangle
{
public:
	float x;
	float y;
	float width;
	float height;
	int id;
	Rectangle(float x, float y, float width, float height)
	{
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
		this->id = -1;
	}
	Rectangle(int id, float x, float y, float width, float height)
	{
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
		this->id = id;
	}
	Rectangle()
	{
		this->x = 0;
		this->y = 0;
		this->width = 0;
		this->height = 0;
		this->id = -1;
	}
};