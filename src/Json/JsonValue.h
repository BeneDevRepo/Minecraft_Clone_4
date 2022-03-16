#pragma once

#include "Json_defaults.h"

#include <string>


JSON_NAMESPACE_START

class Value {
	union {
		std::string sVal;
		int iVal;
		Object* oVal;
		Array* aVal;
		bool bVal;
	};

public:
	enum class Type { String, Int, Object, Array, Bool, Null } type;

public:
	inline Value():					type(Type::Null) { } // Null Object
	inline Value(std::string sVal):	type(Type::String),	sVal(sVal) { }
	inline Value(int iVal):			type(Type::Int),	iVal(iVal) { }
	inline Value(Object* oVal):		type(Type::Object),	oVal(oVal) { }
	inline Value(Array* aVal):		type(Type::Array),	aVal(aVal) { }
	inline Value(bool bVal):		type(Type::Bool),	bVal(bVal) { }
	inline ~Value() {
		free();
	}

	Value(const Value &other);
	Value &operator=(const Value &other);

private:
	void free();

public:
	inline Type getType()	const { return type; }

	inline std::string	getString()	const { assert(type == Type::String);	return sVal; }
	inline int 			getInt()	const { assert(type == Type::Int);		return iVal; }
	inline Object&		getObject()	const { assert(type == Type::Object);	return *oVal; }
	inline Array&		getArray()	const { assert(type == Type::Array);	return *aVal; }
	inline bool 		getBool()	const { assert(type == Type::Bool);		return bVal; }

	// inline Value& operator[](const ObjectKey& key) const { assert(type == Type::Object);	return (*oVal)[key]; }
	// inline Value& operator[](const size_t index) const { assert(type == Type::Array);	return (*aVal)[index]; }
	Value& operator[](const ObjectKey& key) const;
	Value& operator[](const size_t index) const;
};


JSON_NAMESPACE_END