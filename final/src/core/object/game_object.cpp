#include "core/object/game_object.hpp"
#include <stdexcept>

namespace tanks {
GameObject::GameObject() { link(transform_); }

GameObject::~GameObject() = default;

void GameObject::attachComponent(Component &component) {
	if (component.owner_)
		throw std::logic_error("component already has an owner");

	requireMissing(typeid(component));

	link(component);
}

void GameObject::requireMissing(const std::type_info &type) const {
	for (const Component *item = first_; item; item = item->next_)
		if (typeid(*item) == type)
			throw std::logic_error("component type is already attached");
}

void GameObject::link(Component &component) {
	component.owner_ = this;

	if (last_)
		last_->next_ = &component;
	else
		first_ = &component;

	last_ = &component;
}
} // namespace tanks
