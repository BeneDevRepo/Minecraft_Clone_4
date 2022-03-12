#pragma once

#include "Json_defaults.h"

#include "JsonValue.h"

#include <unordered_map>
#include <string>


JSON_NAMESPACE_START

class Object {
	using Key = std::string;
	std::unordered_map<Key, Value> children;

public:
	Object() = default;
	~Object() = default;

public:
	bool has(const Key& key) const;
	Value& operator[](const Key& key);
	Value operator[](const Key& key) const;
	const std::unordered_map<Key, Value> &getDict() const;
};

JSON_NAMESPACE_END