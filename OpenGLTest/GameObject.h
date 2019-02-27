#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <vector>

#include "Renderable.h"

class GameObject {
	public:
		GameObject();
		~GameObject();

		glm::vec3 getPosition();
		float getLocalPositionY();
		glm::vec2 getLocalPositionXZ();
		glm::vec3 getLocalPosition();
		void setLocalPositionY(float y);
		void setLocalPositionXZ(glm::vec2 xz);
		void setLocalPosition(glm::vec3 position);
		void setParentPosition(glm::vec3 position);

		glm::vec3 getLocalRotation();
		void setLocalRotation(glm::vec3 rotation);
		void setParentRotation(glm::quat rotation);

		float getScale();
		float getLocalScale();
		void setLocalScale(float scale);
		void setParentScale(float scale);

		void addRenderable();
		void addChild(GameObject* child);
		void updateRenderableTransforms();
		void clearRenderablePreviousTransforms();

		std::shared_ptr<Renderable> renderable;
		std::vector<GameObject*> children;
	protected:
		glm::vec3 _localPosition;
		glm::vec3 _position;
		glm::vec3 _parentPosition;

		glm::quat _localRotation;
		glm::vec3 _localRotationEuler;
		glm::quat _rotation;
		glm::quat _parentRotation;

		float _localScale;
		float _scale;
		float _parentScale;
};
