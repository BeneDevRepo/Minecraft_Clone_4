#version 460 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in; // Local Workgroup Size

// layout(rgba32f, binding = 0) uniform image2D img_output;
layout(r11f_g11f_b10f, binding = 0) uniform readonly image2D img_input;
layout(r11f_g11f_b10f, binding = 1) uniform writeonly image2D img_output;

// Builtins:
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint  gl_LocalInvocationIndex;
// const uvec3 gl_WorkGroupSize; // compile-time constant dependant on layout

uniform float cutoff;
uniform float cutoffWidth;

uniform ivec2 dimensions;

void main() {
	// ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy); // get index in global work group i.e x,y position
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.x % dimensions.x, gl_GlobalInvocationID.x / dimensions.x);

	vec3 inputColor = imageLoad(img_input, pixel_coords).rgb;

	imageStore(img_output, pixel_coords, vec4(inputColor * smoothstep(cutoff-cutoffWidth/2., cutoff+cutoffWidth/2., dot(inputColor, inputColor)), 1.));
}