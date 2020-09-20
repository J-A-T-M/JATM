#pragma once
#include <assimp/scene.h>
#include "Renderable.h"

class AssetLoader {
	public:
		static void preloadAssets();
		static Model models[NUM_MODELS];
		static Texture textures[NUM_TEXTURES];
	private:
		static Model loadModel(std::string const &path);
		static Model processMesh(aiMesh * mesh);
		static Texture loadTexture(std::string const & path, bool sRGB = true);
};