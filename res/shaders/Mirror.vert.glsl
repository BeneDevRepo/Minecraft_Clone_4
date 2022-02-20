#version 460 core

layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec2 aUVs;
layout (location = 2) in vec3 aNormal;

// Uniform Buffer Object:
layout (std140, binding = 0) uniform Matrices {
						// Base alignment   // Aligned Offset	
    mat4 projection;	// 16 				// 0   (column 0)
						// 16 				// 16  (column 1)
						// 16 				// 32  (column 2)
						// 16 				// 48  (column 3)

    mat4 view;			// 16 				// 64  (column 0)
						// 16 				// 80  (column 1)
						// 16 				// 96  (column 2)
						// 16 				// 112 (column 3)
};

uniform mat4 model;

out vec3 normal;
// out vec2 UVs;
out vec3 fragPos;

void main() {
	normal = mat3(transpose(inverse(model))) * aNormal;
	// UVs = aUVs;
	fragPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = projection * view * vec4(fragPos, 1.);
}