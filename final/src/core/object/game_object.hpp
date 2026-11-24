#pragma once
#include "core/object/transform.hpp"
#include <memory>
#include <typeinfo>
#include <utility>
#include <vector>

namespace tanks {
class GameObject {
	public:
	GameObject();
	virtual ~GameObject();
	GameObject(const GameObject &) = delete;
	GameObject &operator=(const GameObject &) = delete;

	Transform &transform() { return transform_; }

	const Transform &transform() const { return transform_; }

	bool active() const { return active_; }

	void setActive(bool active) { active_ = active; }

	Component *firstComponent() { return first_; }

	const Component *firstComponent() const { return first_; }

	template <class T, class... Args> T &addComponent(Args &&...args) {
		requireMissing(typeid(T));
		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		T &result = *component;
		ownedComponents_.push_back(std::move(component));
		link(result);

		return result;
	}

	template <class T> T *getComponent() {
		T *match = nullptr;

		for (Component *item = first_; item; item = item->next_)
			if (auto *found = dynamic_cast<T *>(item))
				match = found;

		return match;
	}

	template <class T> const T *getComponent() const {
		const T *match = nullptr;

		for (const Component *item = first_; item; item = item->next_)
			if (auto *found = dynamic_cast<const T *>(item))
				match = found;

		return match;
	}

	protected:
	void attachComponent(Component &component);

	private:
	void requireMissing(const std::type_info &type) const;
	void link(Component &component);

	Transform transform_;
	bool active_ = true;
	Component *first_ = nullptr;
	Component *last_ = nullptr;
	std::vector<std::unique_ptr<Component>> ownedComponents_;
};
} // namespace tanks
