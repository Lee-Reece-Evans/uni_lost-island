#version 330

// Input Variables (received from Vertex Shader)
in vec2 texCoord0;

// Uniform: The Texture
uniform sampler2D texture0;
uniform vec2 resolution = vec2(1280, 720);

const float edgeThreshold = 0.05;
const vec3 lum = vec3(0.299, 0.587, 0.114);	// B&W filter

// Output Variable (sent down through the Pipeline)
out vec4 outColor;


void main(void) 
{
	float s00 = dot(lum, texture(texture0, texCoord0 + vec2(-1,  1) / resolution).rgb);
	float s01 = dot(lum, texture(texture0, texCoord0 + vec2( 0,  1) / resolution).rgb);
	float s02 = dot(lum, texture(texture0, texCoord0 + vec2( 1,  1) / resolution).rgb);
	float s10 = dot(lum, texture(texture0, texCoord0 + vec2(-1,  0) / resolution).rgb);
	float s12 = dot(lum, texture(texture0, texCoord0 + vec2( 1,  0) / resolution).rgb);
	float s20 = dot(lum, texture(texture0, texCoord0 + vec2(-1, -1) / resolution).rgb);
	float s21 = dot(lum, texture(texture0, texCoord0 + vec2( 0, -1) / resolution).rgb);
	float s22 = dot(lum, texture(texture0, texCoord0 + vec2( 1, -1) / resolution).rgb);

	float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
	float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);

	float s = sx *sx + sy * sy;

	if (s > edgeThreshold)
		outColor = vec4(1.0);
	else
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
}
