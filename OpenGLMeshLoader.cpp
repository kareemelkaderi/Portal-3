#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)
#define _CRT_SECURE_NO_WARNINGS
#define TO_RADIANS 3.141592/180.0

int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;

int scene1Height = 25;
float spikeHeight = 24.9;
bool spikeForward = true;

class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 15.0f, float eyeY = 5.0f, float eyeZ = 15.0f, float centerX = -15.0f, float centerY = 10.0f, float centerZ = -15.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
};

Camera camera;

bool isFPV = false;
int ww, hh;
float xangle = 0.0f, yangle = 1.0f, zangle = -1.0f; // Camera angles
float x_pos = 0.0f, y_pos = 0.0f; // Camera yaw
float sensitivity = 0.005f;


// Model Variables
Model_3DS model_house;
Model_3DS model_spike;

// Textures
GLTexture tex_ground;
GLTexture tex_wall;
GLTexture tex_lava;
GLTexture tex_ceiling;
GLTexture tex_spike;

float playerX = 0;
float playerY = 4;
float playerZ = 7.5;
bool keystates[256];

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	//gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.8, 0, 0);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_lava.texture[0]);	// Bind the lava texture

	int lavaRepetitions = 6;
	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-15, -5, -5);
	glTexCoord2f(lavaRepetitions, 0);
	glVertex3f(15, -5, -5);
	glTexCoord2f(lavaRepetitions, lavaRepetitions);
	glVertex3f(15, -5, 5);
	glTexCoord2f(0, lavaRepetitions);
	glVertex3f(-15, -5, 5);
	glEnd();
	glPopMatrix();


	


	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture
	glColor3f(1, 1, 1);	// Dim the ground texture a bit

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-15, 0, -15);
	glTexCoord2f(4, 0);
	glVertex3f(-15, 0, -5);
	glTexCoord2f(4, 4);
	glVertex3f(15, 0, -5);
	glTexCoord2f(0, 4);
	glVertex3f(15, 0, -15);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(15, 0, 15);
	glTexCoord2f(4, 0);
	glVertex3f(15, 0, 5);
	glTexCoord2f(4, 4);
	glVertex3f(-15, 0, 5);
	glTexCoord2f(0, 4);
	glVertex3f(-15, 0, 15);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void RenderCeiling() {
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ceiling.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-15, scene1Height, -15);
	glTexCoord2f(10, 0);
	glVertex3f(-15, scene1Height, 15);
	glTexCoord2f(10, 10);
	glVertex3f(15, scene1Height, 15);
	glTexCoord2f(0, 10);
	glVertex3f(15, scene1Height, -15);
	glEnd();
	glPopMatrix();

	glColor3f(1, 1, 1);
}

void RenderPlayer() {
	float dx = playerX - (playerX - xangle);
	float dz = playerZ - (playerZ - zangle);
	float angle = atan2(dx, dz) * (180.0f / 3.141592);
	glColor3f(0, 0, 1);
	glPushMatrix();
	glTranslatef(playerX, playerY, playerZ);
	glScalef(1, 4, 1);
	glRotatef(angle, 0, 1, 0);
	glTranslatef(0, 0.5, 0);
	glutSolidCube(1);
	glPopMatrix();
}

void drawSpike() {

	glColor3f(1,0 , 0);
	glPushMatrix();
	glTranslatef(0, spikeHeight,-4.5);
	glScalef(15,scene1Height,10);
	glTranslatef(0, 0.5, 0);
	glutSolidCube(1);
	glPopMatrix();

	glColor3f(1, 1, 1);
	glPushMatrix();
	glTranslatef(0, spikeHeight-2.5,0);
	glRotatef(90, 0, 0, 1);
	glScalef(0.4, 0.4, 0.4);
	model_spike.Draw();
	//glutSolidCone(1, 2, 40, 40);
	glPopMatrix();
}

void RenderSpike() {
	//glColor3f(1, 1, 1);
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 15; j++) {
			glPushMatrix();
			glTranslatef(j*2-14,0,i*2-14);
			drawSpike();
			glPopMatrix();
		}
	}
	glColor3f(1, 1, 1);
}

void RenderWall() {
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_wall.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-15, 0, -15);
	glTexCoord2f(4, 0);
	glVertex3f(-15, 0, 15);
	glTexCoord2f(4, 4);
	glVertex3f(-15, scene1Height, 15);
	glTexCoord2f(0, 4);
	glVertex3f(-15, scene1Height, -15);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(15, 0, -15);
	glTexCoord2f(4, 0);
	glVertex3f(15, 0, 15);
	glTexCoord2f(4, 4);
	glVertex3f(15, scene1Height, 15);
	glTexCoord2f(0, 4);
	glVertex3f(15, scene1Height, -15);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-15, 0, 15);
	glTexCoord2f(4, 0);
	glVertex3f(15, 0, 15);
	glTexCoord2f(4, 4);
	glVertex3f(15, scene1Height, 15);
	glTexCoord2f(0, 4);
	glVertex3f(-15, scene1Height, 15);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-15, 0, -15);
	glTexCoord2f(4, 0);
	glVertex3f(15, 0, -15);
	glTexCoord2f(4, 4);
	glVertex3f(15, scene1Height, -15);
	glTexCoord2f(0, 4);
	glVertex3f(-15, scene1Height, -15);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-15, -5, -5);
	glTexCoord2f(4, 0);
	glVertex3f(-15, 0, -5);
	glTexCoord2f(4, 4);
	glVertex3f(-15, 0, 5);
	glTexCoord2f(0, 4);
	glVertex3f(-15, -5, 5);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(15, -5, -5);
	glTexCoord2f(4, 0);
	glVertex3f(15, 0, -5);
	glTexCoord2f(4, 4);
	glVertex3f(15, 0, 5);
	glTexCoord2f(0, 4);
	glVertex3f(15, -5, 5);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(15, -5, -5);
	glTexCoord2f(4, 0);
	glVertex3f(15, 0, -5);
	glTexCoord2f(4, 4);
	glVertex3f(-15, 0, -5);
	glTexCoord2f(0, 4);
	glVertex3f(-15, -5, -5);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(15, -5, 5);
	glTexCoord2f(4, 0);
	glVertex3f(15, 0, 5);
	glTexCoord2f(4, 4);
	glVertex3f(-15, 0, 5);
	glTexCoord2f(0, 4);
	glVertex3f(-15, -5, 5);
	glEnd();
	glPopMatrix();

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//setupCamera();
	glLoadIdentity();

	if (isFPV) {
		gluLookAt(playerX, playerY, playerZ, playerX + xangle, playerY - yangle, playerZ + zangle, 0.0f, playerY, 0.0f);
	}
	else {
		float cameraOffsetX = -3.0f * xangle; 
		float cameraOffsetY = 4.0f + yangle;
		float cameraOffsetZ = -3.0f * zangle;

		float cameraX = playerX + cameraOffsetX;
		float cameraY = playerY + cameraOffsetY;
		float cameraZ = playerZ + cameraOffsetZ;
		gluLookAt(cameraX, cameraY, cameraZ, playerX, playerY + 4, playerZ, 0.0f, playerY, 0.0f);
		RenderPlayer();
	}


	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	// Draw Ground
	RenderGround();

	RenderWall();

	RenderCeiling();

	RenderSpike();


	// Draw Tree Model
	//glPushMatrix();
	//glTranslatef(0, 0, 0);
	//glScalef(0.1, 0.1, 0.1);
	//model_tree.Draw();
	//glPopMatrix();

	// Draw house Model
	//glPushMatrix();
	//model_house.rot.x = 90.f;
	//model_house.Draw();
	//glPopMatrix();

	// Draw Wall Model
	//glPushMatrix();
	//glTranslatef(10,0,10);
	//glRotatef(90.f, 1, 0, 0);
	//model_wall.Draw();
	//glPopMatrix();


	//sky box
	glPushMatrix();

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);


	glPopMatrix();



	glutSwapBuffers();
}

//=======================================================================
// Keyboard Function
//=======================================================================

void KeyboardDown(unsigned char button, int x, int y) {
	keystates[button] = true;

	switch (button) {
	case 'f':
		isFPV = true;
		break;
	case 't':
		isFPV = false;
		break;
	}
}

void KeyboardUp(unsigned char button, int x, int y) {
	keystates[button] = false;
}

//=======================================================================
// Motion Function
//=======================================================================
//void myMotion(int x, int y)
//{
//	y = HEIGHT - y;
//
//	if (cameraZoom - y > 0)
//	{
//		Eye.x += -0.1;
//		Eye.z += -0.1;
//	}
//	else
//	{
//		Eye.x += 0.1;
//		Eye.z += 0.1;
//	}
//
//	cameraZoom = y;
//
//	glLoadIdentity();	//Clear Model_View Matrix
//
//	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters
//
//	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
//	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
//
//	glutPostRedisplay();	//Re-draw scene 
//}

//=======================================================================
// Mouse Function
//=======================================================================
//void myMouse(int button, int state, int x, int y)
//{
//	y = HEIGHT - y;
//
//	if (state == GLUT_DOWN)
//	{
//		cameraZoom = y;
//	}
//}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_house.Load("Models/house/house.3DS");
	//model_tree.Load("Models/mytree/tree1.3ds");
	model_spike.Load("Models/mace/MACE.3ds");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	tex_wall.Load("Textures/concrete.bmp");
	tex_ceiling.Load("Textures/ceiling.bmp");
	tex_lava.Load("Textures/lava.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

void Mouse(int xx, int yy) {
	bool goingUp = false;
	x_pos = (xx - ww / 2) * sensitivity;
	y_pos = ((yy / (hh / 180.0)) - 90) * TO_RADIANS;

	xangle = sin(x_pos);
	yangle = sin(y_pos);
	zangle = -cos(x_pos);
}

void Special(int key, int x, int y) {
	float a = 1.0;

	switch (key) {
	case GLUT_KEY_UP:
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		camera.rotateY(-a);
		break;
	}

	glutPostRedisplay();
}

void movementTimer(int value) {
	float speed = 0.1;
	if (keystates['w']) { // Forward
		playerX += xangle * speed;
		playerZ += zangle * speed;
	}
	if (keystates['s']) { // Backward
		playerX -= xangle * speed;
		playerZ -= zangle * speed;
	}
	if (keystates['d']) { // Right
		playerX -= zangle * speed;
		playerZ += xangle * speed;
	}
	if (keystates['a']) { // Left
		playerX += zangle * speed;
		playerZ -= xangle * speed;
	}

	glutPostRedisplay();
	glutTimerFunc(16, movementTimer, 0);
}

void spikeTimer(int value) {
	if (spikeHeight <= 2.5) {
		spikeForward = false;
	}
	if (spikeHeight >= 24.9) {
		spikeForward = true;
	}
	if (spikeForward) {
		spikeHeight -= 0.02;
	}
	else {
		spikeHeight += 0.02;
	}
	glutPostRedisplay();
	glutTimerFunc(10, spikeTimer, 0);
}

void resize(int w, int h) {

	// Setting ratio and new window size
	float ratio = w * 1.0 / h;
	ww = w;
	hh = h;

	// Setting the viewport
	glViewport(0, 0, w, h);

	// Setting the projection 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set the correct perspective.
	gluPerspective(45.0f, ratio, 0.1f, 100.0f);

	//glOrtho(-10.0, 10.0, -10.0, 10.0, 0.0, 10.0); 

	// Resettign the modelview matrix 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Redisplay the scene
	glutPostRedisplay();

}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowSize(1500, 1000);

	glutInitWindowPosition(0, 0);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutReshapeFunc(resize);
	glutKeyboardFunc(KeyboardDown);
	glutKeyboardUpFunc(KeyboardUp);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutSpecialFunc(Special);

	glutTimerFunc(0, movementTimer, 0);
	glutTimerFunc(0, spikeTimer, 0);
	glutPassiveMotionFunc(Mouse);

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}