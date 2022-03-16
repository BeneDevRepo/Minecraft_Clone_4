#pragma once

#include <cassert>
#include <string>

#define JSON_NAMESPACE_START namespace Json {
#define JSON_NAMESPACE_END   };

JSON_NAMESPACE_START

class Array;
class Object; // unordered Dictionary
class Value;

using ObjectKey = std::string;

JSON_NAMESPACE_END