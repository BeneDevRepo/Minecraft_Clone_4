#pragma once

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "OpenGL/opengl_include.h"
#include "OpenGL/ShaderProgram.h"
#include "OpenGL/Texture.h"

#include "Font/FontMeta.h"

#include <string>
#include <vector>
#include <algorithm>


class TextRenderer {
private:
	GLuint VAO;

	GLuint VBO, EBO;

	FontMeta fontMeta;

public:
	Texture texture;

private:
	static constexpr GLsizeiptr MAX_CHARACTERS_BUFFER = 100; // Number of characters the GPU Buffer has space for

private:
	struct Text {
		std::string text;
		glm::vec2 pos;
		// glm::vec2 scale;
		float scale;
	};
	std::vector<Text> texts;

public:
	TextRenderer(const char *const fontFolder);
	~TextRenderer();

	inline void clear() {
		texts.clear();
	}

	inline void addText(const std::string& text, const glm::vec2& pos, const float scale) {
		texts.push_back({text, pos, scale});
	}

	inline void draw(const ShaderProgram &shader) {
		glBindVertexArray(VAO);

		static struct __attribute__((packed)) Vertex {
			float x, y, u, v;
		} vertices[4 * MAX_CHARACTERS_BUFFER];


		uint32_t numCharsBuffered = 0;

		const auto flush = [&numCharsBuffered, this] () {
				glNamedBufferSubData(VBO, 0 * 4 * sizeof(Vertex), numCharsBuffered * 4 * sizeof(Vertex), vertices);
				glDrawElements(GL_TRIANGLES, numCharsBuffered * 6, GL_UNSIGNED_INT, 0);
				numCharsBuffered = 0;
			};

		for(const Text& text : texts) {
			const size_t numChars = text.text.length();
			const float scale = text.scale / fontMeta.textSize;

			glm::vec2 pos = text.pos;

			for(int charIndex = 0; charIndex < numChars; ++charIndex) {
				CharMeta &chInfo = fontMeta[text.text[charIndex]];

				for(int vertIndex = 0; vertIndex < 4; vertIndex++) {
					Vertex &vert = vertices[(numCharsBuffered) * 4 + vertIndex];

					const uint8_t x = vertIndex % 2;
					const uint8_t y = vertIndex / 2;

					vert.x = pos.x + (-chInfo.origin.x + chInfo.texSize.x * x) * scale;
					vert.y = pos.y + (-chInfo.origin.y + chInfo.texSize.y * y) * scale + 32;

					vert.u = (chInfo.texCoords.x + chInfo.texSize.x * x) * 1. / fontMeta.width;
					vert.v = (chInfo.texCoords.y + chInfo.texSize.y * y) * 1. / fontMeta.height;
				}

				pos.x += chInfo.advance * scale;

				if(++numCharsBuffered == MAX_CHARACTERS_BUFFER) flush();
			}
		}

		if(numCharsBuffered > 0) flush();

		glBindVertexArray(0);
	}
};