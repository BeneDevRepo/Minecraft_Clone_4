#pragma once

#include "Json_defaults.h"

#include "JsonValue.h"

#include <unordered_map>
#include <string>


JSON_NAMESPACE_START

class Object {
	std::unordered_map<ObjectKey, Value> children;

public:
	Object() = default;
	~Object() = default;

public:
	bool has(const ObjectKey& key) const;
	Value& operator[](const ObjectKey& key);
	Value operator[](const ObjectKey& key) const;
	const std::unordered_map<ObjectKey, Value> &getDict() const;
};

JSON_NAMESPACE_END