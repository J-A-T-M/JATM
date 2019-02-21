#include "GameObject.h"
#include <iostream>
#include "EventManager.h"

GameObject::GameObject() {
	renderable = nullptr;
	children = {};

	_localPosition = glm::vec3(0);
	_position = glm::vec3(0);
	_parentPosition = glm::vec3(0);

	_localRotation = glm::identity<glm::quat>();
	_localRotationEuler = glm::vec3(0);
	_rotation = glm::identity<glm::quat>();
	_parentRotation = glm::identity<glm::quat>();

	_localScale = 1.0f;
	_scale = 1.0f;
	_parentScale = 1.0f;
}

GameObject::~GameObject() {
	renderable.reset();
	for (GameObject* child : children) {
		delete child;
	}
	children.clear();
}

#pragma region PositionStuff
glm::vec3 GameObject::getPosition() {
	return _position;
}

glm::vec3 GameObject::getLocalPosition() {
	return _localPosition;
}

void GameObject::setLocalPosition(glm::vec2 xz) {
	_localPosition.x = xz.x;
	_localPosition.z = xz.y;
	_position = _parentPosition + _parentRotation * _localPosition * _parentScale;
	for (GameObject* child : children) {
		child->setParentPosition(_position);
	}
}

void GameObject::setLocalPosition(glm::vec3 position) {
	_localPosition = position;
	_position = _parentPosition + _parentRotation * _localPosition * _parentScale;
	for (GameObject* child : children) {
		child->setParentPosition(_position);
	}
}

void GameObject::setParentPosition(glm::vec3 position) {
	_parentPosition = position;
	_position = _parentPosition + _parentRotation * _localPosition * _parentScale;
	for (GameObject* child : children) {
		child->setParentPosition(_position);
	}
}
#pragma endregion

#pragma region RotationStuff
glm::vec3 GameObject::getLocalRotation() {
	return _localRotationEuler;
}

void GameObject::setLocalRotation(glm::vec3 rotation) {
	_localRotationEuler = rotation;
	_localRotation = glm::quat(_localRotationEuler);
	_rotation = _parentRotation * _localRotation;
	for (GameObject* child : children) {
		child->setParentRotation(_rotation);
	}
}

void GameObject::setParentRotation(glm::quat rotation) {
	_parentRotation = rotation;
	_rotation = _parentRotation * _localRotation;
	_position = _parentPosition + _parentRotation * _localPosition * _parentScale;
	for (GameObject* child : children) {
		child->setParentRotation(_rotation);
	}
}
#pragma endregion

#pragma region ScaleStuff
float GameObject::getScale() {
	return _scale;
}

float GameObject::getLocalScale() {
	return _localScale;
}

void GameObject::setLocalScale(float scale) {
	_localScale = scale;
	_scale = _localScale * _parentScale;
	for (GameObject* child : children) {
		child->setParentScale(_scale);
	}
}

void GameObject::setParentScale(float scale) {
	_parentScale = scale;
	_scale = _localScale * _parentScale;
	_position = _parentPosition + _parentRotation * _localPosition * _parentScale;
	for (GameObject* child : children) {
		child->setParentScale(_scale);
	}
}
#pragma endregion

void GameObject::addRenderable() {
	renderable.reset();
	renderable = std::make_shared<Renderable>();
}

void GameObject::addChild(GameObject* child) {
	children.push_back(child);
}

void GameObject::updateRenderableTransforms() {
	if (renderable != nullptr) {
		renderable->previousPosition = renderable->position;
		renderable->previousRotation = renderable->rotation;
		renderable->previousScale = renderable->scale;

		renderable->position = _position;
		renderable->rotation = _rotation;
		renderable->scale = _scale;
	}

	for (GameObject* child : children) {
		child->updateRenderableTransforms();
	}
}
