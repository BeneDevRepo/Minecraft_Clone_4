#include "JsonObject.h"

// #include "JsonValue.h"

JSON_NAMESPACE_START

bool Object::has(const Key& key) const {
	return children.find(key) != children.end();
}

Value& Object::operator[](const Key& key) {
	// assert(has(key));
	return children[key]; // creates Value for key if it doesn't exist yet
}

Value Object::operator[](const Key& key) const {
	const auto it = children.find(key);
	assert(it != children.end());
	return it->second;
}

const std::unordered_map<Object::Key, Value> &Object::getDict() const {
	return children;
}

JSON_NAMESPACE_END