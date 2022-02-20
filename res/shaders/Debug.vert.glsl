#version 460 core

layout (location = 0) in vec3 aPos;

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

void main() {
	vec3 fragPos = vec3(vec4(aPos, 1.0));

	gl_Position = projection * view * vec4(fragPos, 1.);
}