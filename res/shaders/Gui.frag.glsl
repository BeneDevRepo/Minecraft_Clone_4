#version 460 core

in vec2 UVs;

uniform sampler2DArray font;

out vec4 FragColor;


float sampleFont(float val, float targetDist, float borderWidth) {
	// return smoothstep(targetDist, targetDist - borderWidth, val);
	return smoothstep(targetDist + borderWidth, targetDist, val);
}

void main() {
	// const float borderWidth = .01;
	const float borderWidth = .1;

	// float borderDistX = min(gl_FragCoord.x, 1. - gl_FragCoord.x);
	// float borderDistY = min(gl_FragCoord.y, 1. - gl_FragCoord.y);
	// float borderDist = min(borderDistX, borderDistY);

	float val = 1. - texture(font, vec3(UVs, 0.)).r;

	// float fac0 = sampleFont(val, .6, borderWidth);
	float fac0 = sampleFont(val, .5, borderWidth);
	float fac1 = (1.-fac0) * sampleFont(val, .7, borderWidth);

	vec3 col = vec3(0.);
	col += vec3(1., 0., 0.) * fac0;
	col += vec3(0., 1., 0.) * fac1;

	float alpha = 0.;
	alpha += fac0;
	alpha += fac1;

	FragColor = vec4(col, alpha);

	// float borderFac = smoothstep(.02, .0, borderDist);
	float borderFac = .2;
	FragColor = (1.-borderFac) * FragColor + borderFac * vec4(0., 1., 0., 1.);

	// FragColor = vec4(vec3(1., 0., 0.), fac0);

	// FragColor = vec4(vec3(1.-val), 1.);
}