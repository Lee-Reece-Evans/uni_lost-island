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
#include "fire.h"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D Models
C3dglTerrain terrain, water;
C3dglSkyBox skybox;
C3dglModel Deadtree, Firtree, Tree, Treestump, Cabin,
Camp, Stone, Boat, Dock, Fence, Table, Grave, Plane, Shark, Eagle;

// post processing used in second pass
GLuint nQuad;						// postprocessing overlay
GLuint WImage = 1280, HImage = 720;	// image size to cover screen

// texture ids
GLuint idTexGrass, idTexSand, idTexVolc;	// terrain
GLuint idTexWater;							// water
GLuint idTexWood, idTexStump, idTexFirbranch, idTexLeaf;	// tree
GLuint idTexTable;							// table
GLuint idTexFence;							// fence
GLuint idTexPlane;							// plane
GLuint idTexEagle;							// Eagle
GLuint idTexGrave, idTexGraveNormal;		// gravestone
GLuint idTexCamp, idTexCampNormal;			// campfire
GLuint idTexCabin, idTexCabinNormal;		// cabin
GLuint idTexStone, idTexStoneNormal;		// stone
GLuint idTexBoat, idTexBoatNormal;			// boat
GLuint idTexSharkbody, idTexSharkbodyNormal;	//shark
GLuint idTexNone;							// no texture
GLuint idTexScreen;							// Screen Space Texture (used in the 2nd pass)
GLuint idTexCube;							// cubemap

// The FBO object for post processing
GLuint idFBO;

// GLSL Objects (Shader Programs)
C3dglProgram ProgramBasic;
C3dglProgram ProgramWater;
C3dglProgram ProgramTerrain;
C3dglProgram ProgramEffect;

// Water height
float waterLevel = 1.0f;

// camera position (for first person type camera navigation)
mat4 matrixView;			// The View Matrix
float angleTilt = 15.f;		// Tilt Angle
vec3 cam(0);				// Camera movement values

void reshape(int w, int h);
bool bindImage(string path);

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramBasic.Create()) return false;
	if (!ProgramBasic.Attach(VertexShader)) return false;
	if (!ProgramBasic.Attach(FragmentShader)) return false;
	if (!ProgramBasic.Link()) return false;
	if (!ProgramBasic.Use(true)) return false;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/water.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/water.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramWater.Create()) return false;
	if (!ProgramWater.Attach(VertexShader)) return false;
	if (!ProgramWater.Attach(FragmentShader)) return false;
	if (!ProgramWater.Link()) return false;
	if (!ProgramWater.Use(true)) return false;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/terrain.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/terrain.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramTerrain.Create()) return false;
	if (!ProgramTerrain.Attach(VertexShader)) return false;
	if (!ProgramTerrain.Attach(FragmentShader)) return false;
	if (!ProgramTerrain.Link()) return false;
	if (!ProgramTerrain.Use(true)) return false;

	// post processing
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/effect.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/colour.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramEffect.Create()) return false;
	if (!ProgramEffect.Attach(VertexShader)) return false;
	if (!ProgramEffect.Attach(FragmentShader)) return false;
	if (!ProgramEffect.Link()) return false;
	if (!ProgramEffect.Use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(ProgramBasic.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(ProgramBasic.GetAttribLocation("aNormal"));

	// load 3D models
	if (!terrain.loadHeightmap("models\\heightmap.png", 10)) return false;
	if (!water.loadHeightmap("models\\watermap.png", 10)) return false;
	if (!Deadtree.load("models\\Tree_Dry_1.obj")) return false;
	if (!Cabin.load("models\\cabin\\WoodenCabinObj.obj")) return false;
	if (!Camp.load("models\\camp\\Fire_Pit.obj")) return false;
	if (!Stone.load("models\\stone\\stone.obj")) return false;
	if (!Boat.load("models\\boat\\OldBoat.obj")) return false;
	if (!Dock.load("models\\dock\\WoodenDock.obj")) return false;
	if (!Fence.load("models\\fence\\fence.obj")) return false;
	if (!Table.load("models\\table\\side_table.obj")) return false;
	if (!Grave.load("models\\grave\\Tombstone_RIP.obj")) return false;
	if (!Firtree.load("models\\firtree\\Fir.obj")) return false;
	if (!Tree.load("models\\tree\\Tree.obj")) return false;
	if (!Treestump.load("models\\treestump\\treestump.obj")) return false;
	if (!Plane.load("models\\plane\\plane.obj")) return false;
	if (!Shark.load("models\\shark\\Shark.obj")) return false;
	if (!Eagle.load("models\\eagle\\Eagle.obj")) return false;

	// skybox
	if (!skybox.load("skyboxes\\FullMoon\\FullMoonFront2048.png",
		"skyboxes\\FullMoon\\FullMoonLeft2048.png",
		"skyboxes\\FullMoon\\FullMoonBack2048.png",
		"skyboxes\\FullMoon\\FullMoonRight2048.png",
		"skyboxes\\FullMoon\\FullMoonUp2048.png",
		"skyboxes\\FullMoon\\FullMoonDown2048.png")) return false;

	// create & load textures
    glActiveTexture(GL_TEXTURE0);
	// Grass texture
	glGenTextures(1, &idTexGrass);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	bindImage("models\\grass.png");

	// Sand texture
	glGenTextures(1, &idTexSand);
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	bindImage("models\\sand.png");

	// rocky texture
	glGenTextures(1, &idTexVolc);
	glBindTexture(GL_TEXTURE_2D, idTexVolc);
	bindImage("models\\rock.jpg");

	// Water texture
	glGenTextures(1, &idTexWater);
	glBindTexture(GL_TEXTURE_2D, idTexWater);
	bindImage("models\\water.png");

	// fence
	glGenTextures(1, &idTexFence);
	glBindTexture(GL_TEXTURE_2D, idTexFence);
	bindImage("models\\fence/fence_texture.jpg");
	
	// tree
	glGenTextures(1, &idTexWood);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	bindImage("models\\wood.png");

	//stump
	glGenTextures(1, &idTexStump);
	glBindTexture(GL_TEXTURE_2D, idTexStump);
	bindImage("models\\treestump\\treestump.jpg");

	// treeleaf
	glGenTextures(1, &idTexLeaf);
	glBindTexture(GL_TEXTURE_2D, idTexLeaf);
	bindImage("models\\tree\\leaf.png");

	// fir branch
	glGenTextures(1, &idTexFirbranch);
	glBindTexture(GL_TEXTURE_2D, idTexFirbranch);
	bindImage("models\\firtree\\FirBranches_Df.png");

	// table
	glGenTextures(1, &idTexTable);
	glBindTexture(GL_TEXTURE_2D, idTexTable);
	bindImage("models\\table\\walnut.png");

	// eagle
	glGenTextures(1, &idTexEagle);
	glBindTexture(GL_TEXTURE_2D, idTexEagle);
	bindImage("models\\eagle\\eagle.png");

	// plane
	glGenTextures(1, &idTexPlane);
	glBindTexture(GL_TEXTURE_2D, idTexPlane);
	bindImage("models\\plane\\plane.jpg");

	// gravestone
	glGenTextures(1, &idTexGrave);
	glBindTexture(GL_TEXTURE_2D, idTexGrave);
	bindImage("models\\grave\\Standard_D.jpg");

	// campfire
	glGenTextures(1, &idTexCamp);
	glBindTexture(GL_TEXTURE_2D, idTexCamp);
	bindImage("models\\camp\\Fire_Pit_Texture.png");

	// cabin
	glGenTextures(1, &idTexCabin);
	glBindTexture(GL_TEXTURE_2D, idTexCabin);
	bindImage("models\\cabin\\WoodCabinDif.jpg");

	// stone texture
	glGenTextures(1, &idTexStone);
	glBindTexture(GL_TEXTURE_2D, idTexStone);
	bindImage("models\\stone\\stone.png");
	
	// boat texture
	glGenTextures(1, &idTexBoat);
	glBindTexture(GL_TEXTURE_2D, idTexBoat);
	bindImage("models\\boat\\boattex.jpg");
	
	// boat texture
	glGenTextures(1, &idTexSharkbody);
	glBindTexture(GL_TEXTURE_2D, idTexSharkbody);
	bindImage("models\\shark\\greatwhiteshark.png");

	// none (simple-white) texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);

	// Send the texture info to the shaders
	ProgramBasic.SendUniform("texture0", 0);
	ProgramTerrain.SendUniform("texture0", 0);
	ProgramWater.SendUniform("texture0", 0);
	ProgramEffect.SendUniform("texture0", 0);

	// setup the multi textures for shore and lake bed
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	ProgramTerrain.SendUniform("textureBed", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	ProgramTerrain.SendUniform("textureShore", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, idTexVolc);
	ProgramTerrain.SendUniform("textureVolc", 3);

	// normal mapping textures
	glActiveTexture(GL_TEXTURE4);

	// cabin normal
	glGenTextures(1, &idTexCabinNormal);
	glBindTexture(GL_TEXTURE_2D, idTexCabinNormal);
	bindImage("models\\cabin\\WoodCabinNM.jpg");
	
	//camp normal
	glGenTextures(1, &idTexCampNormal);
	glBindTexture(GL_TEXTURE_2D, idTexCampNormal);
	bindImage("models\\camp\\Normal_Mapping.bmp");
	
	// stone normal
	glGenTextures(1, &idTexStoneNormal);
	glBindTexture(GL_TEXTURE_2D, idTexStoneNormal);
	bindImage("models\\stone\\normal.png");
	
	// boat normal
	glGenTextures(1, &idTexBoatNormal);
	glBindTexture(GL_TEXTURE_2D, idTexBoatNormal);
	bindImage("models\\boat\\boattexnm.jpg");

	// gravestone normal
	glGenTextures(1, &idTexGraveNormal);
	glBindTexture(GL_TEXTURE_2D, idTexGraveNormal);
	bindImage("models\\grave\\Standard_N.jpg");

	// shark normal
	glGenTextures(1, &idTexSharkbodyNormal);
	glBindTexture(GL_TEXTURE_2D, idTexSharkbodyNormal);
	bindImage("models\\shark\\greatwhiteshark_n.png");

	// send to shader
	ProgramBasic.SendUniform("textureNormal", 4);

	// load Cube Map
	glActiveTexture(GL_TEXTURE5);
	glGenTextures(1, &idTexCube);
	glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	ProgramBasic.SendUniform("textureCubeMap", 5);

	// reset to 0 as default
	glActiveTexture(GL_TEXTURE0);

	// setup the water colours and level
	ProgramWater.SendUniform("waterColor", 0.2f, 0.22f, 0.02f);
	ProgramWater.SendUniform("skyColor", 0.05f, 0.05f, 0.08f);
	ProgramTerrain.SendUniform("waterColor", 0.2f, 0.22f, 0.02f);
	ProgramTerrain.SendUniform("waterLevel", waterLevel);

	// water fog
	ProgramTerrain.SendUniform("fogDensity", 0.3);

	// setup lights (for basic and terrain programs only, water does not use these lights):
	ProgramBasic.SendUniform("lightAmbient.on", 1);
	ProgramBasic.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	ProgramBasic.SendUniform("lightDir.on", 1);
	ProgramBasic.SendUniform("lightDir.direction", -1.0, 0.5, -1.0);
	ProgramBasic.SendUniform("lightDir.diffuse", 0.4, 0.4, 0.4);
	ProgramTerrain.SendUniform("lightAmbient.on", 1);
	ProgramTerrain.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	ProgramTerrain.SendUniform("lightDir.on", 1);
	ProgramTerrain.SendUniform("lightDir.direction", -1.0, 0.5, -1.0);
	ProgramTerrain.SendUniform("lightDir.diffuse", 0.4, 0.4, 0.4);

	// setup materials (for basic and terrain programs only, water does not use these materials)
	ProgramBasic.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("shininess", 3.0);
	ProgramTerrain.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	ProgramTerrain.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);

	// light point bonfire
	ProgramBasic.SendUniform("lightPoint1.on", 1);
	ProgramBasic.SendUniform("lightPoint1.diffuse", 1.0, 1.0, 1.0);
	ProgramTerrain.SendUniform("lightPoint1.on", 1);
	ProgramTerrain.SendUniform("lightPoint1.diffuse", 1.0, 1.0, 1.0);

	// Create screen space texture - post processing
	glGenTextures(1, &idTexScreen);
	glBindTexture(GL_TEXTURE_2D, idTexScreen);

	// Texture parameters - to get nice filtering 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// This will allocate an uninitilised texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WImage, HImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	
	// Create a framebuffer object (FBO)
	glGenFramebuffers(1, &idFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, idFBO);

	// Attach a depth buffer
	GLuint depth_rb;
	glGenRenderbuffers(1, &depth_rb);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, WImage, HImage);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);

	// attach the texture to FBO depth attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, idTexScreen, 0);

	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	// Create Quad
	float vertices[] = {
		0.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,	1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,	0.0f, 1.0f
	};
	// Generate the buffer name
	glGenBuffers(1, &nQuad);
	// Bind the vertex buffer and send data
	glBindBuffer(GL_ARRAY_BUFFER, nQuad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));
	matrixView *= lookAt(
		vec3(4.0, 0.4, 30.0),
		vec3(4.0, 0.4, 0.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky colour

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	// init Smoke
	if (!initSmoke())
		return false;

	// init fire
	if (!initFire())
		return false;

	return true;
}

void done()
{
}

bool bindImage(string path)
{
	C3dglBitmap bm;
	if (!bm.GetBits()) return false;
	bm.Load(path, GL_RGBA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	return true;
}

void renderModel(mat4 matrixView, float x, float y, float z,
	float rotation, float rotx, float roty, float rotz,
	float scalex, float scaley, float scalez, float Y,
	GLuint texture1, GLuint texture2, string modelname)
{
	mat4 m = matrixView;

	m = translate(m, vec3(x, Y + y, z));
	m = rotate(m, radians(rotation), vec3(rotx, roty, rotz));
	m = scale(m, vec3(scalex, scaley, scalez));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);

	// no normal maps
	if (modelname == "tree")
	{
		Tree.render(0, m); // trunk
		glBindTexture(GL_TEXTURE_2D, texture2);
		Tree.render(1, m); // branches
	}
	else if (modelname == "firtree")
	{
		Firtree.render(2, m); // trunk
		glBindTexture(GL_TEXTURE_2D, texture2);
		Firtree.render(0, m); // branches
	}
	else if (modelname == "deadtree")
	{
		Deadtree.render(m);
	}
	else if (modelname == "treestump")
	{
		Treestump.render(m);
	}
	else if (modelname == "fence")
	{
		Fence.render(m);
	}
	else if (modelname == "plane")
	{
		m = rotate(m, radians(-20.f), vec3(0.0f, 0.0f, 1.0f));
		Plane.render(1, m);
	}
	else if (modelname == "table")
	{
		Table.render(m);
	}
	else if (modelname == "eagle")
	{
		m = translate(m, vec3(60.0f, 0.0f, 0.0f));
		Eagle.render(m);
	}
	// normal maps
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, texture2);
	if (modelname == "cabin")
	{
		Cabin.render(m);
	}
	else if (modelname == "campfire")
	{
		Camp.render(m);
	}
	else if (modelname == "boat")
	{
		Boat.render(m);
	}
	else if (modelname == "dock")
	{
		Dock.render(6, m);
		Dock.render(10, m);
	}
	else if (modelname == "stone")
	{
		Stone.render(m);
	}
	else if (modelname == "grave")
	{
		Grave.render(m);
	}
	else if (modelname == "shark")
	{
		m = translate(m, vec3(135.0f, 0.0f, 0.0f)); // radius relative to starting pos
		Shark.render(m);
	}
	glActiveTexture(GL_TEXTURE0);
}

void renderObjects(mat4 matrixView, float theta, float Y)
{
	ProgramBasic.Use();
	mat4 m = matrixView;

	// skybox render
	ProgramBasic.SendUniform("lightAmbient.color", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("materialDiffuse", 0.0, 0.0, 0.0);
	skybox.render(m);

	// reset lighting
	ProgramBasic.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	ProgramBasic.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);

	// setup the fireplace light
	float light = 0.8f + 0.2f * (float)rand() / (float)RAND_MAX;
	ProgramTerrain.SendUniform("lightPoint1.position", -28.2f, Y + 1.95f, -4.5f);
	ProgramBasic.SendUniform("lightPoint1.position", -28.2f, Y + 1.95f, -4.5f);
	if (rand() % 16 == 0)
	{
		ProgramBasic.SendUniform("lightPoint1.diffuse", light * 1.0, light * 0.35, light * 0.0);
		ProgramTerrain.SendUniform("lightPoint1.diffuse", light * 1.0, light * 0.35, light * 0.0);
	}

	// -----------Normal mapping objects----------------
	ProgramBasic.SendUniform("hasNormal", 1);
	//cabin
	renderModel(matrixView, -29.0f, 1.765f, -5.0f, 40.0f, 0.0f, 1.0f, 0.0f, 0.015f, 0.015f, 0.015f, Y, idTexCabin, idTexCabinNormal, "cabin");

	//campfire
	renderModel(matrixView, -28.2f, 1.74f, -4.5f, 5.0f, 1.0f, 1.0f, 0.0f, 0.05f, 0.05f, 0.05f, Y, idTexCamp, idTexCampNormal, "campfire");

	// boat
	renderModel(matrixView, -20.5f, 1.1f, 10.3f, 45.0f, 0.0f, 1.0f, 0.0f, 0.03f, 0.03f, 0.03f, Y, idTexBoat, idTexBoatNormal, "boat");

	// dock
	renderModel(matrixView, -23.8f, 0.6f, 13.5f, 180.0f, 0.0f, 1.0f, 0.0f, 0.04f, 0.04f, 0.04f, Y, idTexBoat, idTexBoatNormal, "dock");

	// gravestone
	renderModel(matrixView, -25.4f, 1.55f, -7.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.03f, 0.03f, 0.03f, Y, idTexGrave, idTexGraveNormal, "grave");
	renderModel(matrixView, -25.0f, 1.55f, -7.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.03f, 0.03f, 0.03f, Y, idTexGrave, idTexGraveNormal, "grave");

	// stone
	renderModel(matrixView, -28.3f, 1.72f, -5.8f, 30.0f, 1.0f, 0.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexStone, idTexStoneNormal, "stone");
	renderModel(matrixView, -28.5f, 1.72f, -5.4f, 0.0f, 0.0f, 1.0f, 0.0f, 0.003f, 0.003f, 0.003f, Y, idTexStone, idTexStoneNormal, "stone");
	renderModel(matrixView, -27.3f, 1.75f, 7.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.005f, 0.005f, 0.005f, Y, idTexStone, idTexStoneNormal, "stone");

	// shark 1
	renderModel(matrixView, -13.5f, 0.7f, 13.5f, -theta, 0.0f, 1.0f, 0.0f, 0.05f, 0.05f, 0.05f, Y, idTexSharkbody, idTexSharkbodyNormal, "shark");
	renderModel(matrixView, 16.0f, 0.7f, 15.0f, 0.8* -theta, 0.0f, 1.0f, 0.0f, 0.05f, 0.05f, 0.05f, Y, idTexSharkbody, idTexSharkbodyNormal, "shark");

	ProgramBasic.SendUniform("hasNormal", 0);
	glActiveTexture(GL_TEXTURE0);
	// -----------No normal mapping objects----------------
	// table
	renderModel(matrixView, -28.7f, 1.83f, -4.1f, 0.0f, 0.0f, 1.0f, 0.0f, 0.005f, 0.005f, 0.005f, Y, idTexTable, idTexNone, "table");

	// plane
	renderModel(matrixView, -15.0f, 4.3f, -19.0f, 80.0f, 0.0f, 1.0f, 0.0f, 0.25f, 0.25f, 0.25f, Y, idTexPlane, idTexNone, "plane");

	// eagle
	renderModel(matrixView, 0.0f, 10.5f, -2.0f, -theta, 0.0f, 1.0f, 0.0f, 0.1f, 0.1f, 0.1f, Y, idTexEagle, idTexNone, "eagle");
	
	// fence back panels
	renderModel(matrixView, -25.4f, 1.68f, -8.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexFence, idTexNone, "fence");
	renderModel(matrixView, -25.0f, 1.68f, -8.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexFence, idTexNone, "fence");
	renderModel(matrixView, -24.6f, 1.68f, -8.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexFence, idTexNone, "fence");

	// left panels
	renderModel(matrixView, -25.6f, 1.68f, -7.8f, 90.0f, 0.0f, 1.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexFence, idTexNone, "fence");
	renderModel(matrixView, -25.6f, 1.68f, -7.4f, 90.0f, 0.0f, 1.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexFence, idTexNone, "fence");

	// right panels
	renderModel(matrixView, -24.45f, 1.68f, -7.8f, 90.0f, 0.0f, 1.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexFence, idTexNone, "fence");
	renderModel(matrixView, -24.45f, 1.68f, -7.4f, 90.0f, 0.0f, 1.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexFence, idTexNone, "fence");

	// --------------fir tree main island-----------------
	renderModel(matrixView, -19.0f, 1.7f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexWood, idTexFirbranch, "firtree");
	renderModel(matrixView, -28.5f, 1.8f, -12.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexWood, idTexFirbranch, "firtree");
	renderModel(matrixView, -20.0f, 3.3f, -18.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0045f, 0.0045f, 0.0045f, Y, idTexWood, idTexFirbranch, "firtree");
	renderModel(matrixView, -6.0f, 1.5f, -17.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0045f, 0.0045f, 0.0045f, Y, idTexWood, idTexFirbranch, "firtree");

	// (island 1)
	renderModel(matrixView, -10.4f, 2.3f, 15.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexWood, idTexFirbranch, "firtree");
	renderModel(matrixView, -16.5f, 2.6f, 11.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.003f, 0.003f, 0.003f, Y, idTexWood, idTexFirbranch, "firtree");
	renderModel(matrixView, -14.5f, 2.3f, 15.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.005f, 0.005f, 0.005f, Y, idTexWood, idTexFirbranch, "firtree");

	// (island 2)
	renderModel(matrixView, 13.5f, 3.2f, 15.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0035f, 0.0035f, 0.0035f, Y, idTexWood, idTexFirbranch, "firtree");
	renderModel(matrixView, 14.5f, 3.0f, 13.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0045f, 0.0045f, 0.0045f, Y, idTexWood, idTexFirbranch, "firtree");
	renderModel(matrixView, 17.5f, 3.0f, 13.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0045f, 0.0045f, 0.0045f, Y, idTexWood, idTexFirbranch, "firtree");
	renderModel(matrixView, 16.5f, 3.0f, 17.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.004f, 0.004f, 0.004f, Y, idTexWood, idTexFirbranch, "firtree");

	//(island 3)
	renderModel(matrixView, 16.5f, 2.8f, -17.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0045f, 0.0045f, 0.0045f, Y, idTexWood, idTexFirbranch, "firtree");
	renderModel(matrixView, 19.5f, 2.8f, -11.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0035f, 0.0035f, 0.0035f, Y, idTexWood, idTexFirbranch, "firtree");

	// ------------regular trees-------------- (island 1)
	renderModel(matrixView, -13.5f, 2.3f, 12.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.25f, 0.25f, 0.25f, Y, idTexWood, idTexLeaf, "tree");
	renderModel(matrixView, -12.0f, 2.3f, 14.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.35f, 0.35f, 0.35f, Y, idTexWood, idTexLeaf, "tree");
	renderModel(matrixView, -23.5f, 3.1f, -18.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.35f, 0.35f, 0.35f, Y, idTexWood, idTexLeaf, "tree");
	renderModel(matrixView, -27.0f, 2.1f, 11.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.3f, 0.3f, 0.3f, Y, idTexWood, idTexLeaf, "tree");

	// camp tree
	renderModel(matrixView, -29.8f, 1.73f, -4.3f, 0.0f, 0.0f, 1.0f, 0.0f, 0.3f, 0.3f, 0.3f, Y, idTexWood, idTexLeaf, "tree");

	// main island
	renderModel(matrixView, -21.0f, 1.7f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.25f, 0.25f, 0.25f, Y, idTexWood, idTexLeaf, "tree");
	renderModel(matrixView, -14.5f, 2.2f, -8.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.37f, 0.37f, 0.37f, Y, idTexWood, idTexLeaf, "tree");

	// ------------dead trees--------------
	renderModel(matrixView, -25.4f, 1.58f, -9.0f, 40.0f, 0.0f, 1.0f, 0.0f, 0.0002f, 0.0002f, 0.0002f, Y, idTexWood, idTexNone, "deadtree");
	renderModel(matrixView, -26.7f, 1.58f, -8.0f, 70.0f, 0.0f, 1.0f, 0.0f, 0.00028f, 0.00028f, 0.00028f, Y, idTexWood, idTexNone, "deadtree");
	renderModel(matrixView, -23.4f, 1.58f, -7.8f, 25.0f, 0.0f, 1.0f, 0.0f, 0.00025f, 0.00025f, 0.00025f, Y, idTexWood, idTexNone, "deadtree");
	renderModel(matrixView, -24.0f, 1.58f, -11.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0002f, 0.0002f, 0.0002f, Y, idTexWood, idTexNone, "deadtree");
	renderModel(matrixView, -22.6f, 1.58f, -10.0f, 15.0f, 0.0f, 1.0f, 0.0f, 0.00023f, 0.00023f, 0.00023f, Y, idTexWood, idTexNone, "deadtree");
	
	// ------------tree stumps--------------
	renderModel(matrixView, -24.2f, 1.65f, -9.0f, -90.0f, 1.0f, 0.0f, 0.0f, 0.095f, 0.095f, 0.095f, Y, idTexStump, idTexNone, "treestump");
	renderModel(matrixView, -25.5f, 1.65f, -10.5f, -90.0f, 1.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.1f, Y, idTexStump, idTexNone, "treestump");
	renderModel(matrixView, -27.2f, 1.65f, -9.5f, -90.0f, 1.0f, 0.0f, 0.0f, 0.085f, 0.085f, 0.085f, Y, idTexStump, idTexNone, "treestump");
	renderModel(matrixView, -24.0f, 1.65f, -6.5f, -90.0f, 1.0f, 0.0f, 0.0f, 0.09f, 0.09f, 0.09f, Y, idTexStump, idTexNone, "treestump");
	renderModel(matrixView, -25.8f, 1.65f, -6.1f, -90.0f, 1.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.1f, Y, idTexStump, idTexNone, "treestump");

	renderModel(matrixView, -25.2f, 2.19f, 7.3f, -90.0f, 1.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.1f, Y, idTexStump, idTexNone, "treestump");

	// ------render the terrain-------
	ProgramTerrain.Use();
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	m = translate(matrixView, vec3(0, Y, 0));
	terrain.render(m);

	// ----------render the water---------
	ProgramWater.Use();
	glBindTexture(GL_TEXTURE_2D, idTexWater);
	m = translate(matrixView, vec3(0, Y + waterLevel, 0));
	m = scale(m, vec3(0.65f, 1.0f, 0.65f));
	ProgramWater.SendUniform("matrixModelView", m);
	water.render(m);

	// ----------render the particle effects---------
	renderSmoke(matrixView, translate(matrixView, vec3(0.5, Y + 9.2, -3.0)), 750);
	renderFire(matrixView, translate(matrixView, vec3(-28.2f, Y + 1.72f, -4.5f)), 120);
}

void renderTeapot(mat4 ma, float theta, float Y)
{
	ProgramBasic.Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	glActiveTexture(GL_TEXTURE5);
	ProgramBasic.SendUniform("materialDiffuse", 0.8, 0.2, 0.2);
	ProgramBasic.SendUniform("materialAmbient", 0.8, 0.2, 0.2);
	ProgramBasic.SendUniform("materialSpecular", 1.0, 0.5, 0.5);

	mat4 m = matrixView;
	m = translate(m, vec3(-28.7f, Y + 1.855f, -4.1f));
	m = rotate(m, radians(-80.f), vec3(0, 1, 0));
	ProgramBasic.SendUniform("matrixModelView", m);
	glutSolidTeapot(0.02);

	glActiveTexture(GL_TEXTURE0);
	ProgramBasic.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("materialSpecular", 0.0, 0.0, 0.0);
}

void prepareCubeMap(float x, float y, float z, float theta, float Y)
{
	// Store the current viewport in a safe place
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int w = viewport[2];
	int h = viewport[3];

	// setup the viewport to 256x256, 90 degrees FoV (Field of View)
	glViewport(0, 0, 256, 256);
	ProgramBasic.SendUniform("matrixProjection", perspective(radians(90.f), 1.0f, 0.02f, 1000.0f));
	ProgramTerrain.SendUniform("matrixProjection", perspective(radians(90.f), 1.0f, 0.02f, 1000.0f));
	reshapeSmoke(perspective(radians(90.f), 1.0f, 0.02f, 1000.0f));
	reshapeFire(perspective(radians(90.f), 1.0f, 0.02f, 1000.0f));

	// render environment 6 times
	mat4 tmp = matrixView;	// store matrixView
	ProgramBasic.SendUniform("reflectionPower", 0.0);
	for (int i = 0; i < 6; ++i)
	{
		// clear background
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// setup the camera
		const GLfloat ROTATION[6][6] =
		{	// at              up
			{ 1.0, 0.0, 0.0,   0.0, -1.0, 0.0 },  // pos x
			{ -1.0, 0.0, 0.0,  0.0, -1.0, 0.0 },  // neg x
			{ 0.0, 1.0, 0.0,   0.0, 0.0, 1.0 },   // pos y
			{ 0.0, -1.0, 0.0,  0.0, 0.0, -1.0 },  // neg y
			{ 0.0, 0.0, 1.0,   0.0, -1.0, 0.0 },  // poz z
			{ 0.0, 0.0, -1.0,  0.0, -1.0, 0.0 }   // neg z
		};
		mat4 matrixView = lookAt(
			vec3(x, y, z),
			vec3(x + ROTATION[i][0], y + ROTATION[i][1], z + ROTATION[i][2]),
			vec3(ROTATION[i][3], ROTATION[i][4], ROTATION[i][5]));

		// send the View Matrix
		ProgramBasic.SendUniform("matrixView", matrixView);

		// render scene objects - all but the reflective one
		glActiveTexture(GL_TEXTURE0);
		renderObjects(matrixView, theta, Y);

		// send the image to the cube texture
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
		glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, 0, 0, 256, 256, 0);
	}

	// restore the matrixView, viewport and projection
	matrixView = tmp;
	void reshape(int w, int h);
	reshape(w, h);
}

void render()
{
	// this global variable controls the animation
	float theta = glutGet(GLUT_ELAPSED_TIME) * 0.01f;

	// send the animation time to shaders
	ProgramWater.SendUniform("t", glutGet(GLUT_ELAPSED_TIME) / 1000.f);

	glBindFramebufferEXT(GL_FRAMEBUFFER, idFBO);

	// calculate the Y position of the camera - above the ground
	float Y = -std::max(terrain.getInterpolatedHeight(inverse(matrixView)[3][0], inverse(matrixView)[3][2]), waterLevel);

	// create cube map
	prepareCubeMap(-28.7f, Y + 1.88f, -4.1f, theta, Y);

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
	matrixView = m * matrixView;

	// setup View Matrix
	ProgramTerrain.SendUniform("matrixView", matrixView);
	ProgramBasic.SendUniform("matrixView", matrixView);
	
	// render objects
	ProgramBasic.SendUniform("reflectionPower", 0.0);
	renderObjects(matrixView, theta, Y);
	ProgramBasic.SendUniform("reflectionPower", 0.7);
	renderTeapot(matrixView, theta, Y);

	// -----------Pass 2: on-screen rendering--------------
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	ProgramEffect.Use();

	// setup ortographic projection
	ProgramEffect.SendUniform("matrixProjection", ortho(0, 1, 0, 1, -1, 1));

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, idTexScreen);

	// setup identity matrix as the model-view
	ProgramEffect.SendUniform("matrixModelView", mat4(1));

	GLuint attribVertex = ProgramEffect.GetAttribLocation("aVertex");
	GLuint attribTextCoord = ProgramEffect.GetAttribLocation("aTexCoord");
	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribTextCoord);
	glBindBuffer(GL_ARRAY_BUFFER, nQuad);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glVertexAttribPointer(attribTextCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribTextCoord);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void reshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 m = perspective(radians(60.f), ratio, 0.02f, 1000.f);

	ProgramBasic.SendUniform("matrixProjection", m);
	ProgramTerrain.SendUniform("matrixProjection", m);
	ProgramWater.SendUniform("matrixProjection", m);

	reshapeSmoke(perspective(radians(60.f), ratio, 0.02f, 1000.f));
	reshapeFire(perspective(radians(60.f), ratio, 0.02f, 1000.f));
}

bool changeShader(char *pVertexShader, char *pFragmentShader)
{
	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile(pVertexShader)) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile(pFragmentShader)) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramEffect.Create()) return false;
	if (!ProgramEffect.Attach(VertexShader)) return false;
	if (!ProgramEffect.Attach(FragmentShader)) return false;
	if (!ProgramEffect.Link()) return false;
	if (!ProgramEffect.Use(true)) return false;
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': cam.z = std::max(cam.z * 1.05f, 0.01f); break;
	case 's': cam.z = std::min(cam.z * 1.05f, -0.01f); break;
	case 'a': cam.x = std::max(cam.x * 1.05f, 0.01f); break;
	case 'd': cam.x = std::min(cam.x * 1.05f, -0.01f); break;
	case 'e': cam.y = std::max(cam.y * 1.05f, 0.01f); break;
	case 'q': cam.y = std::min(cam.y * 1.05f, -0.01f); break;
	case '1': changeShader("shaders/effect.vert", "shaders/colour.frag"); return;
	case '2': changeShader("shaders/effect.vert", "shaders/negative.frag"); return;
	case '3': changeShader("shaders/effect.vert", "shaders/sepia.frag"); return;
	case '4': changeShader("shaders/effect.vert", "shaders/edge.frag"); return;
	case '5': changeShader("shaders/effect.vert", "shaders/blur.frag"); return;
	case '6': changeShader("shaders/effect.vert", "shaders/pixelate.frag"); return;
	case '7': changeShader("shaders/effect.vert", "shaders/fisheye.frag"); return;
	case '8': changeShader("shaders/effect.vert", "shaders/dream.frag"); return;
	}
	// speed limit
	cam.x = std::max(-0.15f, std::min(0.15f, cam.x));
	cam.y = std::max(-0.15f, std::min(0.15f, cam.y));
	cam.z = std::max(-0.15f, std::min(0.15f, cam.z));
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': cam.z = 0; break;
	case 'a':
	case 'd': cam.x = 0; break;
	case 'q':
	case 'e': cam.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
bool bJustClicked = false;
void onMouse(int button, int state, int x, int y)
{
	bJustClicked = (state == GLUT_DOWN);
	glutSetCursor(bJustClicked ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

// handle mouse move
void onMotion(int x, int y)
{
	if (bJustClicked)
		bJustClicked = false;
	else
	{
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// find delta (change to) pan & tilt
		float deltaPan = 0.25f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
		float deltaTilt = 0.25f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// View = Tilt * DeltaPan * Tilt^-1 * DeltaTilt * View;
		angleTilt += deltaTilt;
		mat4 m = mat4(1.f);
		m = rotate(m, radians(angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaPan), vec3(0.f, 1.f, 0.f));
		m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaTilt), vec3(1.f, 0.f, 0.f));
		matrixView = m * matrixView;
	}
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WImage, HImage);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}

