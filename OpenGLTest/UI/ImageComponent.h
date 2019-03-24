#pragma once

#include "AssetLoader.h"
#include "UIComponent.h"

class ImageComponent : public UIComponent {
	public:
		ImageComponent(std::string imagePath, float width, float height, float x, float y);
		bool IsTransparent();
};