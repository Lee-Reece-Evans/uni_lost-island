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
	vec4 color = vec4(0, 0, 0, 1);
	outColor = color;
    outColor += texture(texture0, texCoord0.st);
    outColor = vec4(1 - outColor.r, 1 - outColor.g, 1 - outColor.b, 1);
}
