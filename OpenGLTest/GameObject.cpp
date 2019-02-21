#include "GameObject.h"
#include <iostream>
#include "EventManager.h"

GameObject::GameObject() {
	_localPosition = glm::vec3(0);
	_position = glm::vec3(0);
	_parentPosition = glm::vec3(0);

	renderable = nullptr;
}

GameObject::~GameObject() {
	renderable.reset();
	for (GameObject* child : _children) {
		delete child;
	}
	_children.clear();
}

glm::vec3 GameObject::getPosition() {
	return _position;
}

glm::vec3 GameObject::getLocalPosition() {
	return _localPosition;
}

void GameObject::setLocalPosition(glm::vec2 xz) {
	_localPosition.x = xz.x;
	_localPosition.z = xz.y;
	_position = _parentPosition + _localPosition;
	for (GameObject* child : _children) {
		child->setParentPosition(_position);
	}
}

void GameObject::setLocalPosition(glm::vec3 position) {
	_localPosition = position;
	_position = _parentPosition + _localPosition;
	for (GameObject* child : _children) {
		child->setParentPosition(_position);
	}
}

void GameObject::setParentPosition(glm::vec3 position) {
	_parentPosition = position;
	_position = _parentPosition + _localPosition;
	for (GameObject* child : _children) {
		child->setParentPosition(_position);
	}
}

void GameObject::addRenderable() {
	renderable.reset();
	renderable = std::make_shared<Renderable>();
}

void GameObject::addChild(GameObject* gameObject) {
	_children.push_back(gameObject);
}

void GameObject::updateRenderableTransforms() {
	if (renderable != nullptr) {
		renderable->previousPosition = renderable->position;
		renderable->position = _position;
	}
	for (GameObject* child : _children) {
		child->updateRenderableTransforms();
	}
}
