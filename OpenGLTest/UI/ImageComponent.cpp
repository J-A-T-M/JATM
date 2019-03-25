#include "ImageComponent.h"

ImageComponent::ImageComponent(std::string imagePath, float width, float height, float x, float y) : UIComponent(width, height, x, y) {
	texture = TEXTURE_UV_GRID;
	aspectRatio = float(AssetLoader::textures[texture].width) / AssetLoader::textures[texture].height;
    color = glm::vec4(1.0f);
}

bool ImageComponent::IsTransparent() { return true; }