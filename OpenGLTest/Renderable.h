#pragma once
//used by Renderer;

#include <vector>

typedef unsigned char GLubyte;
typedef unsigned int GLuint;
typedef float GLfloat;
#include <glm/gtc/quaternion.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "Enums.h"
#include "Colour.h"

struct Camera {
	glm::vec3 position;
	glm::vec3 target;
	GLfloat FOV;
	GLfloat nearClip;
	GLfloat farClip;
};

struct DirectionalLight {
	glm::vec3 direction;
	glm::vec3 color;
	GLfloat nearclip;
	GLfloat	farclip;
};

struct Model {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> UVs;
	std::vector<GLuint> elements;
	GLuint positionLoc;
	GLuint UVLoc;
	GLuint normalLoc;
	GLuint elementLoc;
};

struct Texture {
	std::vector<GLubyte> data;
	int width;
	int height;
	GLuint loc;
};

class Renderable {
	public:
		Renderable(glm::vec3 color = Colour::GREY, ModelEnum model = MODEL_CUBE, TextureEnum texture = TEXTURE_NONE, float roughness = 0.5f, float metallic = 0.0f);

		ModelEnum model;
		TextureEnum texture;

		glm::vec3 pos;
		glm::quat rot;
		glm::vec3 size;
		// transform
		glm::vec3 renderPositionCur;
		glm::quat renderRotationCur;
		glm::vec3 renderScaleCur;
		// stuff to allow renderer to interpolate positions
		bool interpolated;
		glm::vec3 renderPositionPrev;
		glm::quat renderRotationPrev;
		glm::vec3 renderScalePrev;

		// material
		glm::vec3 color;
		bool fullBright;
		float roughness;
		float metallic;
};
