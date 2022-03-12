#include "Json/Json.h"

#include <fstream>
#include <iostream>


JSON_NAMESPACE_START

inline bool whitespace(const char c) {
	return c == ' ' || c == '	' || c == '\r' || c == '\n';
}

inline void skipWhitespace(std::istream &stream) {
	for(;;) {
		char c;
		stream >> c;
		if(!whitespace(c)) {
			stream.unget();
			break;
		}
	}
}

bool parseBool(std::istream &source, bool &success);
int parseInt(std::istream &source, bool &success);
std::string parseString(std::istream &source, bool &success);
Object *parseObject(std::istream &source, bool &success);
Value parseValue(std::istream &source, bool &success);


bool parseBool(std::istream &source, bool &success) {
	skipWhitespace(source);
	constexpr const char *TRUE = "true";
	constexpr const char *FALSE = "false";

	const bool expectedOutcome = source.peek() == TRUE[0];
	const char *expected = expectedOutcome ? TRUE : FALSE;

	char c;

	for(; *expected; ++expected) {
		source >> c;
		assert(c == *expected);
		if(c != *expected) success = false;
	}

	return expectedOutcome;
}

int parseInt(std::istream &source, bool &success) {
	skipWhitespace(source);

	bool negative = false;
	if(source.peek() == '-') {
		negative = true;
		source.ignore(1);
	}

	int result = 0;
	for(char c; ; ) {
		if(int next = source.peek(); next < '0' || next > '9') break;

		source >> c;
		result = result * 10 + (c - '0');
	}

	return negative ? -result : result;
}

std::string parseString(std::istream &source, bool &success) {
	skipWhitespace(source);

	char c;
	source >> c;
	assert(c == '\"');

	std::string out;

	bool openEscapeSequence = false;

	for(;;) {
		source >> c;

		if(!openEscapeSequence) {
			if(c == '\\') {
				openEscapeSequence = true;
				continue;
			} else if(c == '\"')
				break;
		} else {
			openEscapeSequence = false;
		}

		out += c;
	}

	return out;
}


Object *parseObject(std::istream &source, bool &success) {
	skipWhitespace(source);

	char c;
	source >> c;
	assert(c == '{');

	Object *out = new Object;

	for(;;) {
		skipWhitespace(source);

		switch(source.peek()) {
			case '\"':
				{
					const std::string key = parseString(source, success);

					// printf("Key: %s\n", key.c_str());

					skipWhitespace(source);

					source >> c;
					assert(c == ':');

					skipWhitespace(source);

					Value &newVal = (*out)[key];
					newVal = parseValue(source, success);

					skipWhitespace(source);

					if(source.peek() == ',') source.ignore(1);
				}
				break;

			case '}':
				source.ignore(1);
				return out;
		}
	}

	// const std::string key = parseString(source, success);

	// printf("Parsed string: %s\n", key.c_str());

	// for(;;) {
	// }

	return nullptr;
}

Value parseValue(std::istream &source, bool &success) {
	skipWhitespace(source);

	switch(source.peek()) {
		case '{':
			return Value{parseObject(source, success)};

		case '[':
			break;

		case '\"':
			return Value{parseString(source, success)};

		case 't':
		case 'f':
			return Value{parseBool(source, success)};

		default:
			return Value{parseInt(source, success)}; // parse number
	}
	return {}; // Placeholder (TODO: add unused Json variations)
}

Value fromFile(const char *const filename) {
	std::ifstream file(filename);

	// skipWhitespace(file);

	bool success;
	// parseValue(file, success);

	// char c;
	// while(!file.eof()) {
	// 	file >> c;
	// 	std::cout << c;
	// }

	return parseValue(file, success);
}



template<>
std::string serialize(const Value &val) {
	switch(val.getType()) {
		case Value::Type::Bool:
			return val.getBool() ? "true" : "false";
		case Value::Type::Int:
			return std::to_string(val.getInt());
		case Value::Type::Object:
			return serialize(val.getObject());
		case Value::Type::Null:
			return "null";
	}
	return "<error>";
}


template<>
std::string serialize(const Object &obj) {
	return "";
}

JSON_NAMESPACE_END