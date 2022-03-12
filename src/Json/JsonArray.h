#pragma once

#include "Json_defaults.h"

#include "JsonValue.h"

#include <vector>

JSON_NAMESPACE_START

class Array {
	std::vector<Value> values;

public:
	Array() = default;
	~Array() = default;

public:
	size_t size() const;
	Value& operator[](const size_t index);
	Value operator[](const size_t index) const;
};

JSON_NAMESPACE_END