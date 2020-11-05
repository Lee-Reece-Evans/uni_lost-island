#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;

// Input: Water Related
in float waterDepth;		// water depth (positive for underwater, negative for the shore)

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

// fog
in float fogFactor;

// Uniform: The Texture
uniform sampler2D texture0;
uniform sampler2D textureBed;
uniform sampler2D textureShore;
uniform sampler2D textureVolc;

// uniforms - material parameters
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// view matrix (needed for lighting)
uniform mat4 matrixView;

// Water-related
uniform vec3 waterColor;
uniform vec3 fogColor;

struct POINT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint1;

vec4 PointLight(POINT light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);

	// diffuse light
	vec3 L = normalize(matrixView * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	// specular light
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	// attenuation camp fire
	float att_quadratic = 0.25;
	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = clamp(1 / (att_quadratic * dist * dist), 0, 1);

	return color * att;
}

void main(void) 
{
	outColor = color;

	if (lightPoint1.on == 1)
	{
		outColor += PointLight(lightPoint1);
	}
	
	// shoreline multitexturing
	float isAboveWater = clamp(-waterDepth, 0, 1); 

	// volcano texture
	float isAboveWater2 = clamp(-waterDepth - 2.2, 0, 1);

	if (isAboveWater < 1)
		outColor *= mix(texture(textureBed, texCoord0), texture(textureShore, texCoord0), isAboveWater);
	else // blend grass / rock
		outColor *= mix(texture(textureShore, texCoord0), texture(textureVolc, texCoord0), isAboveWater2);
	
	// fog added to lake
	outColor = mix(vec4(waterColor, 1), outColor, fogFactor);
}