///////////////////////////////////////////////////////////////////////////
// These are implementation of functions responsible for rendering the fire
// YOU DO NOT NEED TO MODIFY THIS CODE

#define _USE_MATH_DEFINES
#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Additional include file
#include "fire.h"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// Shader Program
C3dglProgram ProgramFire;

// Particle System Params
const float PERIOD = 0.009f;
const float LIFETIME = 1.0f;
const int NPARTICLES = (int)(LIFETIME / PERIOD);

// Particle Buffer ids
GLuint idBufferInitialPos;
GLuint idBufferVelocity2;
GLuint idBufferStartTime2;

// Texture Id
GLuint idTexParticle2;

bool initFire()
{
	// enable: transparency/blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// enable: sprite point size = 50
	//glPointSize(50);
	glEnable(0x8642);	// !!!!
	glEnable(GL_POINT_SPRITE);

	// Initialise Shader - Fire
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/fire.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/fire.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramFire.Create()) return false;
	if (!ProgramFire.Attach(VertexShader)) return false;
	if (!ProgramFire.Attach(FragmentShader)) return false;
	if (!ProgramFire.Link()) return false;
	if (!ProgramFire.Use(true)) return false;

	// Prepare the particle buffers
	std::vector<float> bufferVelocity;
	std::vector<float> bufferStartTime;
	std::vector<float> bufferInitialPos;
	float time = 0;
	for (int i = 0; i < NPARTICLES; i++)
	{
		float x, y, z;

		// initial position
		const float r = 0.02;
		float alpha = (float)M_PI * 2.f * (float)rand()/(float)RAND_MAX;
		x = sin(alpha);
		y = 0;
		z = cos(alpha);
		float d = r * (float)rand()/(float)RAND_MAX;

		bufferInitialPos.push_back(d * x);
		bufferInitialPos.push_back(y);
		bufferInitialPos.push_back(d * z);

		// initial velocity
		float theta = (float)M_PI / 1.5f * (float)rand()/(float)RAND_MAX;
		float phi = (float)M_PI * 2.f * (float)rand()/(float)RAND_MAX;
		x = sin(theta) * cos(phi);
		y = cos(theta);
		z = sin(theta) * sin(phi);
		float v = 0.05f + 0.05f * (float)rand()/(float)RAND_MAX;

		bufferVelocity.push_back(x * v);
		bufferVelocity.push_back(y * v);
		bufferVelocity.push_back(z * v);

		// start time
		bufferStartTime.push_back(time);
		time += PERIOD;

	}
	
	glGenBuffers(1, &idBufferInitialPos);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferInitialPos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferInitialPos.size(), &bufferInitialPos[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferVelocity2);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferStartTime2);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0], GL_STATIC_DRAW);

	// Setup the particle system
	ProgramFire.SendUniform("gravity",	 0.0, 0.1, 0.0);
	ProgramFire.SendUniform("particleLifetime", LIFETIME);


	// Setup the Particle Texture
    glActiveTexture(GL_TEXTURE7);
	C3dglBitmap bm("models/fire.png", GL_RGBA);
    glGenTextures(1, &idTexParticle2);
    glBindTexture(GL_TEXTURE_2D, idTexParticle2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	ProgramFire.SendUniform("texture0", 7);

	return true;
}

void renderFire(glm::mat4 matrixView, glm::mat4 matrixModelView, int pSize)
{
	// Switch the Particle Shader program to use
	ProgramFire.Use();

	// setup the Uniform Variables
	ProgramFire.SendUniform("matrixView", matrixView);
	ProgramFire.SendUniform("matrixModelView", matrixModelView);
	ProgramFire.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);
	ProgramFire.SendUniform("particleSize", pSize);

	// disable writes to the depth buffer
	glDepthMask(GL_FALSE);

	// render the buffer
	glEnableVertexAttribArray(0);	// initial position
	glEnableVertexAttribArray(1);	// velocity
	glEnableVertexAttribArray(2);	// start time
	glBindBuffer(GL_ARRAY_BUFFER, idBufferInitialPos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity2);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NPARTICLES);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	// revert to normal
	glDepthMask(GL_TRUE);
}

void reshapeFire(glm::mat4 matrixProjection)
{
	ProgramFire.SendUniform("matrixProjection", matrixProjection);
}

