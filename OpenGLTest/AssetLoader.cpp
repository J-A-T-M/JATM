#include "AssetLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

#include "Enums.h"

Model AssetLoader::models[NUM_MODELS];
Texture AssetLoader::textures[NUM_TEXTURES];

Model AssetLoader::loadModel(std::string const &path) {
	// read file via ASSIMP
	Assimp::Importer importer;
	const unsigned int flags = aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;
	const aiScene* scene = importer.ReadFile(path, flags);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { // if is Not Zero
		std::cout << "ERROR::ASSIMP " << importer.GetErrorString() << std::endl;
		return Model();
	}

	aiMesh& mesh = *scene->mMeshes[0];
	return processMesh(mesh);
}

static inline glm::vec3 vec3_cast(const aiVector3D &v) { return glm::vec3(v.x, v.y, v.z); }
static inline glm::vec2 vec2_cast(const aiVector3D &v) { return glm::vec2(v.x, v.y); }

Model AssetLoader::processMesh(aiMesh &mesh) {
	Model ret;
	for (int i = 0; i < mesh.mNumVertices; ++i) {
		ret.positions.push_back(vec3_cast(mesh.mVertices[i]));
		ret.normals.push_back(vec3_cast(mesh.mNormals[i]));

		if (mesh.HasTextureCoords(0)) {
			ret.UVs.push_back(vec2_cast(mesh.mTextureCoords[0][i]));
			ret.tangents.push_back(vec3_cast(mesh.mTangents[i]));
			ret.bitangents.push_back(-vec3_cast(mesh.mBitangents[i]));
		} else {
			ret.UVs.push_back(glm::vec2(0, 0));
			ret.tangents.push_back(glm::vec3(0, 0, 0));
			ret.bitangents.push_back(glm::vec3(0, 0, 0));
		}
	}
	for (int i = 0; i < mesh.mNumFaces; i++) {
		aiFace face = mesh.mFaces[i];
		for (int j = 0; j < face.mNumIndices; j++) {
			ret.elements.push_back(face.mIndices[j]);
		}
	}
	return ret;
}

Texture AssetLoader::loadTexture(std::string const & path, bool sRGB) {
	Texture texture;
	texture.sRGB = sRGB;
	stbi_set_flip_vertically_on_load(true);
	GLubyte* texData = stbi_load(path.c_str(), &texture.width, &texture.height, NULL, STBI_rgb_alpha);
	texture.data.assign(texData, texData + texture.width * texture.height * 4);
	return texture;
}

void AssetLoader::preloadAssets() {
	models[MODEL_CUBE] = loadModel("../assets/models/cube.obj");
	models[MODEL_CUBE_BEVEL] = loadModel("../assets/models/cube_bevel.obj");
	models[MODEL_SPHERE] = loadModel("../assets/models/sphere.obj");
	models[MODEL_SUZANNE] = loadModel("../assets/models/suzanne.obj");
	textures[TEXTURE_UV_GRID] = loadTexture("../assets/textures/UV Grid.png");
	textures[TEXTURE_FONT_SHARE_TECH_MONO] = loadTexture("../assets/textures/ShareTechMono.png");
	textures[TEXTURE_NORMAL_BRICKS] = loadTexture("../assets/textures/bricks_normal.jpg", false);
	textures[TEXTURE_DISP_BRICKS] = loadTexture("../assets/textures/bricks_disp.jpg", false);
}