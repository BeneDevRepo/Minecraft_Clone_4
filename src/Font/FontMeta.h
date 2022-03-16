#pragma once

#include "Json/Json.h"

#include <cstdint>

// "0":{"x":288,"y":42,"width":27,"height":35,"originX":5,"originY":29,"advance":18},

struct CharMeta {
	struct ivec2 { int32_t x, y; };
	ivec2 texCoords, texSize, origin;
	int32_t advance;

public:
	inline CharMeta():
			texCoords{}, texSize{}, origin{}, advance{} { }
	// inline CharMeta(const ivec2& texCoords, const ivec2& texSize, const ivec2& origin, const uint32_t advance):
	// 		texCoords(texCoords), texSize(texSize), origin(origin), advance(advance) { }
};

class FontMeta {
	CharMeta chars[256];

public:
	uint32_t width, height;
	uint32_t textSize;

public:
	FontMeta(const char *const folder);
	CharMeta& operator[](const char c);
};

