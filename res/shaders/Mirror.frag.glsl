#version 460 core

in vec3 normal;
in vec3 fragPos;

uniform vec3 viewPos;
uniform samplerCube skybox;

out vec4 FragColor;

void main() {
	// Mirror:
    // vec3 I = normalize(fragPos - viewPos);
    // vec3 R = reflect(I, normalize(normal));
    // FragColor = vec4(texture(skybox, R).rgb, 1.0);

	// Transparent:
	/*
	ratio = refIndex_from / refIndex_to
	RenIndices:
	Air: 1.0
	Water: 1.33
	Ice: 1.309
	Glass: 1.52
	Diamond: 2.42
	*/
    float ratio = 1.00 / 1.52; // Air to Glass
    // float ratio = 1.00 / 2.42; // Air to Diamond
    vec3 I = normalize(fragPos - viewPos);
    vec3 R = refract(I, normalize(normal), ratio);
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}