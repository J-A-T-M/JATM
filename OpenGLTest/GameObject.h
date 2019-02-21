#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <vector>

#include "Renderable.h"

class GameObject {
	public:
		GameObject();
		~GameObject();

		glm::vec3 getPosition();
		glm::vec3 getLocalPosition();

		void setLocalPosition(glm::vec2 xz);
		void setLocalPosition(glm::vec3 position);
		void setParentPosition(glm::vec3 position);

		void addRenderable();
		void addChild(GameObject* gameObject);
		void updateRenderableTransforms();

		std::shared_ptr<Renderable> renderable;
	protected:

		glm::vec3 _parentPosition;
		glm::vec3 _position;
		glm::vec3 _localPosition;

		std::vector<GameObject*> _children;
};
