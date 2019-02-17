#pragma once
class Scene {
	public:
	virtual void Setup() = 0;
	virtual void Update(const float delta) = 0;
	virtual bool Done() = 0;
	virtual void Cleanup() = 0;
};
