#include "JsonValue.h"

#include "JsonArray.h"
#include "JsonObject.h"

JSON_NAMESPACE_START


Value::Value(const Value &other): type(other.type) {
	switch(type) {
		case Type::String:
			new(&sVal) std::string;
			sVal = other.sVal;
			break;

		case Type::Object:
			oVal = new Object;
			*oVal = *(other.oVal);
			break;

		case Type::Array:
			aVal = new Array;
			*aVal = *(other.aVal);
			break;

		default:
			memcpy(this, &other, sizeof(Value));
			break;
	}
}

Value &Value::operator=(const Value &other) {
	free();
	this->type = other.type;
	switch(type) {
		case Type::String:
			new(&sVal) std::string;
			sVal = other.sVal;
			break;

		case Type::Object:
			oVal = new Object;
			*oVal = *(other.oVal);
			break;

		case Type::Array:
			aVal = new Array;
			*aVal = *(other.aVal);
			break;

		default:
			memcpy(this, &other, sizeof(Value));
			break;
	}
	return *this;
}

void Value::free() {
	switch(type) {
		case Type::String:
			sVal.~basic_string();
			break;

		case Type::Object:
			delete oVal;
			break;

		case Type::Array:
			delete aVal;
			break;
	}
}

Value& Value::operator[](const ObjectKey& key) const {
	assert(type == Type::Object);
	return (*oVal)[key];
}

Value& Value::operator[](const size_t index) const {
	assert(type == Type::Array);
	return (*aVal)[index];
}

JSON_NAMESPACE_END