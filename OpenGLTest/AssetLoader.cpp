#include "AssetLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

Model AssetLoader::models[NUM_MODELS];
Texture AssetLoader::textures[NUM_TEXTURES];

Model AssetLoader::loadModel(std::string const &path) {
	// read file via ASSIMP
	Assimp::Importer importer;
	const unsigned int flags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;
	const aiScene* scene = importer.ReadFile(path, flags);

	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { // if is Not Zero
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		throw "Division by zero condition!";
	}

	aiMesh* mesh = scene->mMeshes[0];
	return processMesh(scene->mMeshes[0]);
}

Model AssetLoader::processMesh(aiMesh *mesh) {
	Model ret;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		glm::vec3 pos;
		pos.x = mesh->mVertices[i].x;
		pos.y = mesh->mVertices[i].y;
		pos.z = mesh->mVertices[i].z;
		ret.positions.push_back(pos);

		glm::vec3 normal;
		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;
		ret.normals.push_back(normal);

		if (mesh->mTextureCoords[0]) {
			glm::vec2 texCoord;
			texCoord.x = mesh->mTextureCoords[0][i].x;
			texCoord.y = mesh->mTextureCoords[0][i].y;
			ret.UVs.push_back(texCoord);
		} else {
			ret.UVs.push_back(glm::vec2(0, 0));
		}

	}
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			ret.elements.push_back(face.mIndices[j]);
		}
	}
	return ret;
}

Texture AssetLoader::loadTexture(std::string const & path) {
	Texture texture;
	stbi_set_flip_vertically_on_load(true);
	GLubyte* texData = stbi_load(path.c_str(), &texture.width, &texture.height, NULL, 4);
	texture.data.assign(texData, texData + texture.width * texture.height * 4);
	return texture;
}

void AssetLoader::preloadAssets() {
	models[MODEL_CUBE] = loadModel("../assets/models/cube.obj");
	models[MODEL_CUBE_BEVEL] = loadModel("../assets/models/cube_bevel.obj");
	models[MODEL_SPHERE] = loadModel("../assets/models/sphere.obj");
	models[MODEL_SUZANNE] = loadModel("../assets/models/suzanne.obj");
	textures[TEXTURE_UV_GRID] = loadTexture("../assets/textures/UV Grid.png");
}