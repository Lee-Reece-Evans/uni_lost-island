#version 330

in float age;

uniform sampler2D texture0;

out vec4 outColor;

void main()
{
	vec4 texel = texture(texture0, gl_PointCoord);

	// alpha
	float alpha = 1 - texel.r * texel.g * texel.b;
    alpha *= 1 - age;

	// RGB
	vec3 yellow = vec3(1, 1, 0);
	vec3 red = vec3(1, 0, 0);
	outColor = vec4(mix(yellow, red, 1 - pow(1 - age, 1.5)), alpha);
}
