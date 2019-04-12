#include "GameObject.h"
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
	_size = glm::vec3(1.0f);
}

GameObject::~GameObject() {
	renderable->parent = nullptr;
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

float GameObject::getLocalPositionY() {
	return _localPosition.y;
}

glm::vec2 GameObject::getLocalPositionXZ() {
	return glm::vec2(_localPosition.x, _localPosition.z);
}

glm::vec3 GameObject::getLocalPosition() {
	return _localPosition;
}

void GameObject::setLocalPositionY(float y) {
	_localPosition.y = y;
	_position = _parentPosition + _parentRotation * _localPosition * _parentScale;
	for (GameObject* child : children) {
		child->setParentPosition(_position);
	}
}

void GameObject::setLocalPositionXZ(glm::vec2 xz) {
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
glm::quat GameObject::getRotation() {
	return _rotation;
}
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

void GameObject::setSize(glm::vec3 size) {
	_size = size;
}
void GameObject::setSize(float size) {
	_size = glm::vec3(size);
}
glm::vec3 GameObject::getSize() {
	return _size;
}
#pragma endregion

void GameObject::addRenderable(glm::vec3 color, ModelEnum model, TextureEnum texture, float roughness, float metallic) {
	renderable.reset();
	renderable = std::make_shared<Renderable>(color, model, texture, roughness, metallic);
	renderable->parent = this;
	clearRenderableTransforms();
	EventManager::notify(RENDERER_ADD_TO_RENDERABLES, &TypeParam<std::shared_ptr<Renderable>>(renderable));
}

void GameObject::addChild(GameObject* child) {
	children.push_back(child);
	child->setParentPosition(_position);
	child->setParentRotation(_rotation);
	child->setParentScale(_scale);
}

void GameObject::clearRenderableTransforms() {
	if (renderable != nullptr) {
		renderable->currPos = _position;
		renderable->currRot = _rotation;
		renderable->currSize = _scale * _size;
		renderable->prevPos = _position;
		renderable->prevRot = _rotation;
		renderable->prevScale = _scale * _size;
	}

	for (GameObject* child : children) {
		child->clearRenderableTransforms();
	}
}