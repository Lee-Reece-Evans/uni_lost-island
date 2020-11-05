#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in vec3 texCoordCubeMap;
in mat3 matrixTangent;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

vec3 normalNew;

// Uniform: The Texture
uniform sampler2D texture0;
uniform sampler2D textureNormal;
uniform samplerCube textureCubeMap;

// uniforms - material parameters
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// Reflection Power
uniform float reflectionPower;

// view matrix (needed for lighting)
uniform mat4 matrixView;

// used for normalmap lighting
uniform int hasNormal;

struct DIRECTIONAL
{	
	int on;
	vec3 direction;
	vec3 diffuse;
};
uniform DIRECTIONAL lightDir;

vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normalNew, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}

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
	float NdotL = dot(normalNew, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	// specular light
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normalNew);
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
	if (hasNormal == 1)
	{
		normalNew = 2.0* texture(textureNormal, texCoord0).xyz -vec3(1.0, 1.0, 1.0);
		normalNew = normalize(matrixTangent * normalNew);
	}
	else
	{
		normalNew = normal;
	}

	outColor = color;

	if (lightDir.on == 1) 
		outColor += DirectionalLight(lightDir);
	if (lightPoint1.on == 1)
		outColor += PointLight(lightPoint1);

	outColor = mix(outColor * texture(texture0, texCoord0.st), texture(textureCubeMap, texCoordCubeMap), reflectionPower);

	// for tree leaves/branches remove alpha
	if(outColor.a < 1.0)
        discard;
}
