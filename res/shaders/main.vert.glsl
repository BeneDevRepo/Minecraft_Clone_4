#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUVs;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

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

uniform mat4 lightSpaceMatrix; // for shadow Mapping

uniform mat4 model;

out VS_OUT {
	vec2 UVs;
	flat mat3 TBN;
	vec3 fragPos;
	vec4 FragPosLightSpace; // for shadow Mapping
} vs_out;

void main() {
	vs_out.UVs = aUVs;

	// normal = mat3(transpose(inverse(model))) * aNormal;

	vec3 N = normalize(vec3(model * vec4(aNormal, 0.))); // Normal
	vec3 T = normalize(vec3(model * vec4(aTangent, 0.))); // Tangent
	vec3 B = cross(N, T); // Bitangent
	vs_out.TBN = mat3(T, B, N);

	vs_out.fragPos = vec3(model * vec4(aPos, 1.0));

	vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.fragPos, 1.0); // for shadow mapping

	gl_Position = projection * view * vec4(vs_out.fragPos, 1.);
}