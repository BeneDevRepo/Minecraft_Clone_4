#version 460 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uvs;

out vec2 UVs;

void main() {
	gl_Position = vec4(pos.xy, 0., 1.);
	UVs = uvs;
}