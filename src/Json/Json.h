#pragma once

#include "Json_defaults.h"

#include "JsonArray.h"
#include "JsonObject.h"
#include "JsonValue.h"

#include <string>
// #include <new>


// https://www.json.org/json-en.html

JSON_NAMESPACE_START

Value fromFile(const char *const filename);

template<typename T>
std::string serialize(const T& target);

template<>
std::string serialize(const Object& obj);

template<>
std::string serialize(const Value& val);

JSON_NAMESPACE_END