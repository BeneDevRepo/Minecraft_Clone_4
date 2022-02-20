#version 460 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in; // Local Workgroup Size

layout(r11f_g11f_b10f, binding = 0) uniform image2D scene;
layout(r11f_g11f_b10f, binding = 1) uniform readonly image2D bloom;

// Builtins:
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint  gl_LocalInvocationIndex;
// const uvec3 gl_WorkGroupSize; // compile-time constant dependant on layout

uniform ivec2 dimensions;

void main() {
	// ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy); // get index in global work group i.e x,y position
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.x % dimensions.x, gl_GlobalInvocationID.x / dimensions.x);

	vec3 outColor = imageLoad(scene, pixel_coords).rgb + imageLoad(bloom, pixel_coords).rgb;

	imageStore(scene, pixel_coords, vec4(outColor, 1.));
}