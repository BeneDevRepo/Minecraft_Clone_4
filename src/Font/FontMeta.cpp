#include "FontMeta.h"

// void printJson(const Json::Value& val) {
// 	switch(val.getType()) {
// 		case Json::Value::Type::Object:
// 			printf("{");
// 			printf("\n"); // --
// 			for(const auto &[key, value] : val.getObject().getDict()) {
// 				printf("\"%s\": ", key.c_str());
// 				printJson(value);
// 				printf(", ");
// 				printf("\n"); // --
// 			}
// 			printf("}");
// 			break;

// 		case Json::Value::Type::Array:
// 			break; // TODO: HANDLE

// 		case Json::Value::Type::Bool:
// 			printf("\"%s\"", val.getBool() ? "true" : "false");
// 			break;

// 		case Json::Value::Type::Int:
// 			printf("%d", val.getInt());
// 			break;

// 		case Json::Value::Type::String:
// 			printf("\"%s\"", val.getString().c_str());
// 			break;

// 		case Json::Value::Type::Null:
// 			printf("<null>");
// 			break;
// 	}
// };

// "0":{"x":288,"y":42,"width":27,"height":35,"originX":5,"originY":29,"advance":18},

FontMeta::FontMeta(const char *const folder):
		chars{} {
	const std::string jsonPath = std::string(folder) + "/font.json";

	const auto json = Json::fromFile(jsonPath.c_str());

	width = json["width"].getInt();
	height = json["height"].getInt();
	textSize = json["size"].getInt();

	for(const auto &[key, value] : json["characters"].getObject().getDict()) {
		CharMeta &meta = (*this)[key[0]];

		// printf("Char <%s> loaded\n", key.c_str());

		meta.texCoords = { value["x"].getInt(), value["y"].getInt() };
		meta.texSize = { value["width"].getInt(), value["height"].getInt() };
		meta.origin = { value["originX"].getInt(), value["originY"].getInt() };
		meta.advance = value["advance"].getInt();
	}

	// printJson(json);
}

CharMeta& FontMeta::operator[](const char c) {
	return chars[(uint8_t)c];
}