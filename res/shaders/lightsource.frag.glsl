#version 460 core

in vec2 UVs;
in vec3 normal;
in vec3 fragPos;

out vec4 FragColor;

void main() {
    FragColor = vec4(1., 1., .75, 1.);
}