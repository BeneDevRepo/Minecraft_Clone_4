#version 460 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uvs;

uniform vec2 screenSize;

out vec2 UVs;

void main() {
	vec2 posOut = pos.xy / screenSize;

	posOut.y = 1. - posOut.y; // invert y Axis

	posOut = posOut * 2. - vec2(1.);

	gl_Position = vec4(posOut, 0., 1.);

	UVs = uvs;
}