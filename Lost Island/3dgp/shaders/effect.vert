#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixModelView;

uniform vec2 resolution = vec2(1280, 720);

layout (location = 0) in vec3 aVertex;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in vec3 aTangent;
layout (location = 5) in vec3 aBiTangent;

out vec2 texCoord0;

void main(void) 
{
	// calculate position
	vec4 position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	vec3 normal = normalize(mat3(matrixModelView) * aNormal);

	// calculate texture coordinate
	texCoord0 = aTexCoord;

	// normal mapped
	vec3 tangent = normalize(mat3 (matrixModelView) * aTangent);
	vec3 biTangent = normalize (mat3 (matrixModelView) * aBiTangent);
}
