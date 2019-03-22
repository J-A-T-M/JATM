#pragma once
class Scene {
	public:
		Scene() : _done(false) {};

		bool Done() {
			return _done;
		}
		virtual ~Scene() {};
		virtual void Update(const float delta) = 0;
		virtual Scene* GetNext() = 0;
	protected:
		bool _done;
};
