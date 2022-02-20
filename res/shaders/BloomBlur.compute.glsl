#version 460 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in; // Local Workgroup Size

// layout(rgba32f, binding = 0) uniform image2D img_output;
layout(r11f_g11f_b10f, binding = 0) uniform readonly image2D img_input;
layout(r11f_g11f_b10f, binding = 1) uniform writeonly image2D img_output;

uniform bool horizontal;
uniform ivec2 dimensions;

// Builtins:
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint  gl_LocalInvocationIndex;
// const uvec3 gl_WorkGroupSize; // compile-time constant dependant on layout

const int offsets[] = {-3, -2, -1, 0, 1, 2, 3};
const float factors[] = {
	0.106288522,
	0.140321344,
	0.165770069,
	0.175240144,
	0.165770069,
	0.140321344,
	0.106288522};

void main() {
	// ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy); // get index in global work group i.e x,y position
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.x % dimensions.x, gl_GlobalInvocationID.x / dimensions.x);

	ivec2 sampleDir = horizontal ? ivec2(1., 0.) : ivec2(0., 1.);

	vec3 outColor = vec3(0.);

	for(int i = 0; i < 7; i++)
		outColor += imageLoad(img_input, pixel_coords + sampleDir*offsets[i]).rgb * factors[i];
	
	imageStore(img_output, pixel_coords, vec4(outColor, 1.));
}