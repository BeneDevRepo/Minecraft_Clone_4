#include "JsonArray.h"

// #include "JsonValue.h"

JSON_NAMESPACE_START

size_t Array::size() const {
	return values.size();
}

Value& Array::operator[](const size_t index) {
	return values[index];
}

Value Array::operator[](const size_t index) const {
	return values[index];
}

JSON_NAMESPACE_END