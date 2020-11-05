#version 330

// Input Variables (received from Vertex Shader)
in vec2 texCoord0;

// Uniform: The Texture
uniform sampler2D texture0;
uniform vec2 resolution = vec2(1280, 720);

const float PI = 3.1415926535;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
	float aperture = 178.0;
	float apertureHalf = 0.5 * aperture * (PI / 180.0);
	float maxFactor = sin(apertureHalf);

	vec2 uv;
	vec2 xy = 2.0 * texCoord0.st - 1.0;
	float d = length(xy);
	if (d < (2.0-maxFactor))
	{
		d = length(xy * maxFactor);
		float z = sqrt(1.0 - d * d);
		float r = atan(d, z) / PI;
		float phi = atan(xy.y, xy.x);
		uv.x = r * cos(phi) + 0.5;
		uv.y = r * sin(phi) + 0.5;
	}
	else
	{
		uv = texCoord0.st;
	}

	outColor = vec4(0, 0, 0, 1);
	outColor += texture2D(texture0, uv);
}
