#version 460 core

in vec2 UVs;

uniform sampler2D screen;

vec3 aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// float aces(float x) {
//   const float a = 2.51;
//   const float b = 0.03;
//   const float c = 2.43;
//   const float d = 0.59;
//   const float e = 0.14;
//   return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
// }

out vec4 FragColor;
// Internal:
// in gl_FrontFacing
// in gl_FragCoord // screen space coordinates (absolute)
// out gl_FragDepth // manually set fragment depth
void main() {

	// FragColor = vec4(1.0) - texture(screen, UVs);
	FragColor = texture(screen, UVs);
	// FragColor = gl_FragCoord / 800.;
	// FragColor.xyz /= 4.;

	// Pseudobloom:
	// FragColor = vec4(0.);
	// FragColor += texture(screen, UVs+vec2(-1., -1.)*.002);
	// FragColor += texture(screen, UVs+vec2( 0., -1.)*.002);
	// FragColor += texture(screen, UVs+vec2( 1., -1.)*.002);
	// FragColor += texture(screen, UVs+vec2(-1.,  0.)*.002);
	// FragColor += texture(screen, UVs+vec2( 0.,  0.)*.002);
	// FragColor += texture(screen, UVs+vec2( 1.,  0.)*.002);
	// FragColor += texture(screen, UVs+vec2(-1.,  1.)*.002);
	// FragColor += texture(screen, UVs+vec2( 0.,  1.)*.002);
	// FragColor += texture(screen, UVs+vec2( 1.,  1.)*.002);
	// FragColor /= 9.;


	// ----------------- Gamma Correction + Tone Mapping:
	// float gamma = 2.2;
    // FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));

	const float gamma = 2.2;
    vec3 hdrColor = FragColor.rgb;
  
    // // reinhard tone mapping
    // vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

	// // exposure tone mapping
	// float exposure = 1.; // variable, z.B. 1., 5., .1.
    // vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    // ACES tone mapping
    vec3 mapped = aces(hdrColor);

    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}