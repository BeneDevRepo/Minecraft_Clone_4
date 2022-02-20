#include "Cubemap.h"

#include "stb/stb_image.h"


#include <cstdint>
#include <iostream>

#include <vector>
#include <string>

Cubemap::Cubemap(const char *const filename) {
	std::vector<std::string> faces {
		"../res/skybox/right.jpg",
		"../res/skybox/left.jpg",
		"../res/skybox/top.jpg",
		"../res/skybox/bottom.jpg",
		"../res/skybox/front.jpg",
		"../res/skybox/back.jpg"
	};

	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture);

	int nrChannels;
	stbi_set_flip_vertically_on_load(false); // dont flip loaded textures on the y-axis.


	for(uint8_t face = 0; face < faces.size(); face++) {
		uint8_t *const data = stbi_load(faces[face].c_str(), &width, &height, &nrChannels, 0);
		
		if(face == 0) // Only done once for entire Cubemap:
			glTextureStorage2D(texture, 1, GL_RGB8, width, height); // simultaneously specify storage for all levels of a two-dimensional or one-dimensional array texture

		if (!data)
			std::cout << "Failed to load texture" << std::endl;

		glTextureSubImage3D(texture, 0, 0, 0, face, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// glGenerateTextureMipmap(texture);
}

Cubemap::~Cubemap() {
}