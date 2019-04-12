#pragma once

class Rect {
	public:
		Rect();
		Rect(float x, float y, float width, float height);
		Rect(int id, float x, float y, float width, float height);

		float x;
		float y;
		float width;
		float height;
		int id;
};