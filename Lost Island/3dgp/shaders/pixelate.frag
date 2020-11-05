#version 330

// Input Variables (received from Vertex Shader)
in vec2 texCoord0;

// Uniform: The Texture
uniform sampler2D texture0;
uniform vec2 resolution = vec2(1280, 720);

// Output Variable (sent down through the Pipeline)
out vec4 outColor;


void main(void) 
{
	const int SIZE = 7;

	vec2 coords = vec2(ivec2(texCoord0 * resolution / SIZE)) / resolution * SIZE;

	vec3 v = vec3(0, 0, 0);
	int n = 0;
	for (int k = -SIZE; k <= SIZE; k++)
		for (int j = -SIZE; j <= SIZE; j++)
		{
			v += texture(texture0, coords + vec2(k, j) / resolution).rgb;
			n++;
		}

	outColor = vec4(v / n, 1);
}
