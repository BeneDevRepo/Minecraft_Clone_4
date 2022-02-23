#include "Texture.h"

#include "stb/stb_image.h"


#include <cstdint>
#include <iostream>

Texture::Texture(const char *const filename) {
	// glCreateTextures(GL_TEXTURE_2D, 1, &texture); // ---------------
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // magnification (from close): linear
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // minification (from far away)

	GLfloat maxAniso;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
	glTextureParameterf(texture, GL_TEXTURE_MAX_ANISOTROPY, maxAniso); // Anisotropic Filtering

	int nChannels;
	stbi_set_flip_vertically_on_load(false); // dont flip loaded textures on the y-axis.
	uint8_t *const data = stbi_load(filename, &width, &height, &nChannels, 0);

	// std::cout << "Width: " << width << "  Height: " << height << "  Channels: " << nChannels << "\n";

	if (!data)
		std::cout << "Failed to load texture" << std::endl;

	const GLsizei NUM_MIPMAPS = 1;
	if(nChannels == 4) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		// glTextureStorage2D(texture, NUM_MIPMAPS, GL_RGBA8, width, height); // simultaneously specify storage for all levels of a two-dimensional or one-dimensional array texture
		// glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data); // specify a two-dimensional texture subimage --------------------
		glTextureStorage3D(texture, NUM_MIPMAPS, GL_RGBA8, width, height, 1/*depth*/); // simultaneously specify storage for all levels of a two-dimensional or one-dimensional array texture
		glTextureSubImage3D(texture, 0, 0, 0, 0/*zOffset*/, width, height, 1/*depth*/, GL_RGBA, GL_UNSIGNED_BYTE, data); // specify a two-dimensional texture subimage
	} else if(nChannels == 3) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		// glTextureStorage2D(texture, NUM_MIPMAPS, GL_RGBA8, width, height); // simultaneously specify storage for all levels of a two-dimensional or one-dimensional array texture
		// glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data); // specify a two-dimensional texture subimage
		glTextureStorage3D(texture, NUM_MIPMAPS, GL_RGBA8, width, height, 1/*depth*/); // simultaneously specify storage for all levels of a two-dimensional or one-dimensional array texture
		glTextureSubImage3D(texture, 0, 0, 0, 0/*zOffset*/, width, height, 1/*depth*/, GL_RGB, GL_UNSIGNED_BYTE, data); // specify a two-dimensional texture subimage
	} else {
		std::cout << "Texture: Unsupported Number of Color Channels\n";
	}

	glGenerateTextureMipmap(texture);

	stbi_image_free(data);
}

Texture::~Texture() {
}