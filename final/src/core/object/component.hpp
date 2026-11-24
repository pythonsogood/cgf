#pragma once
#include <cassert>

namespace tanks {
class GameObject;

class Component {
	public:
	Component() = default;
	virtual ~Component() = default;
	Component(const Component &) = delete;
	Component &operator=(const Component &) = delete;

	GameObject &owner() {
		assert(owner_);
		return *owner_;
	}

	const GameObject &owner() const {
		assert(owner_);
		return *owner_;
	}

	Component *nextComponent() { return next_; }

	const Component *nextComponent() const { return next_; }

	private:
	friend class GameObject;
	GameObject *owner_ = nullptr;
	Component *next_ = nullptr;
};
} // namespace tanks
