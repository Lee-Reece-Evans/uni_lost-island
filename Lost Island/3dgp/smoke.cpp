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
#include "smoke.h"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// Shader Program
C3dglProgram ProgramSmoke;

// Particle System Params
const float PERIOD = 0.01f;
const float LIFETIME = 6.0f;
const int NPARTICLES = (int)(LIFETIME / PERIOD);

// Particle Buffer ids
GLuint idBufferVelocity;
GLuint idBufferStartTime;

// Texture Id
GLuint idTexParticle;

bool initSmoke()
{
	// enable: transparency/blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// enable: sprite point size = 50
	glEnable(0x8642);	// !!!!
	glEnable(GL_POINT_SPRITE);
	//glPointSize(10);

	// Initialise Shader - Smoke
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/smoke.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/smoke.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramSmoke.Create()) return false;
	if (!ProgramSmoke.Attach(VertexShader)) return false;
	if (!ProgramSmoke.Attach(FragmentShader)) return false;
	if (!ProgramSmoke.Link()) return false;
	if (!ProgramSmoke.Use(true)) return false;

	// Prepare the particle buffers
	std::vector<float> bufferVelocity;
	std::vector<float> bufferStartTime;
	float time = 0;
	for (int i = 0; i < NPARTICLES; i++)
	{
		// initial velocity
		float theta = (float)M_PI / 1.5f * (float)rand()/(float)RAND_MAX;
		float phi = (float)M_PI * 2.f * (float)rand()/(float)RAND_MAX;
		float x = sin(theta) * cos(phi);
		float y = cos(theta);
		float z = sin(theta) * sin(phi);
		float v = 0.2f + 0.3f * (float)rand()/(float)RAND_MAX;

		bufferVelocity.push_back(x * v);
		bufferVelocity.push_back(y * v);
		bufferVelocity.push_back(z * v);

		// start time
		bufferStartTime.push_back(time);
		time += PERIOD;
	}
	glGenBuffers(1, &idBufferVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferStartTime);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0], GL_STATIC_DRAW);

	// Setup the particle system
	ProgramSmoke.SendUniform("initialPos", 0.0, 0.0, 0.0);
	ProgramSmoke.SendUniform("gravity",	 0.0, 0.1, 0.0);
	ProgramSmoke.SendUniform("particleLifetime", LIFETIME);

	// Setup the Particle Texture
    glActiveTexture(GL_TEXTURE6);
	C3dglBitmap bm("models/smoke.png", GL_RGBA);
    glGenTextures(1, &idTexParticle);
    glBindTexture(GL_TEXTURE_2D, idTexParticle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	ProgramSmoke.SendUniform("texture0", 6);

	return true;
}

void renderSmoke(glm::mat4 matrixView, glm::mat4 matrixModelView, int pSize)
{
	// Switch the Particle Shader program to use
	ProgramSmoke.Use();

	// setup the Uniform Variables
	ProgramSmoke.SendUniform("matrixView", matrixView);
	ProgramSmoke.SendUniform("matrixModelView", matrixModelView);
	ProgramSmoke.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);
	ProgramSmoke.SendUniform("particleSize", pSize);

	// disable writes to the depth buffer
	glDepthMask(GL_FALSE);

	// render the buffer
	glEnableVertexAttribArray(0);	// velocity
	glEnableVertexAttribArray(1);	// starttime
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NPARTICLES);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	// revert to normal
	glDepthMask(GL_TRUE);
}

void reshapeSmoke(glm::mat4 matrixProjection)
{
	ProgramSmoke.SendUniform("matrixProjection", matrixProjection);
}

