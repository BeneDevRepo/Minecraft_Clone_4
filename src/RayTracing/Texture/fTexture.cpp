#include "fTexture.h"

fTexture::fTexture(const uint32_t width, const uint32_t height):
		pixels(new uint32_t[width * height]),
		width(width),
		height(height) {
}

fTexture::~fTexture() {
	delete[] pixels;
}