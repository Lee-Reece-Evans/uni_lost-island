#version 330

// Input Variables (received from Vertex Shader)
in vec2 texCoord0;

// Uniform: The Texture
uniform sampler2D texture0;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
	vec2 uv = texCoord0.st;

	outColor = vec4(0, 0, 0, 1);
    outColor += texture(texture0, uv);

	outColor += texture2D(texture0, uv+0.001);
	outColor += texture2D(texture0, uv+0.003);
	outColor += texture2D(texture0, uv+0.005);
	outColor += texture2D(texture0, uv+0.007);
	outColor += texture2D(texture0, uv+0.009);
	outColor += texture2D(texture0, uv+0.011);

	outColor += texture2D(texture0, uv-0.001);
	outColor += texture2D(texture0, uv-0.003);
	outColor += texture2D(texture0, uv-0.005);
	outColor += texture2D(texture0, uv-0.007);
	outColor += texture2D(texture0, uv-0.009);
	outColor += texture2D(texture0, uv-0.011);

	outColor.rgb = vec3((outColor.r + outColor.g + outColor.b) / 3.0) / 9.5;
}
