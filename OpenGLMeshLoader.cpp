#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glut.h>

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)
#define _CRT_SECURE_NO_WARNINGS
#define TO_RADIANS 3.141592/180.0

int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex;
char title[] = "3D Model Loader Sample";

float wallHalfLength = 15.0;
float wallHalfWidth = 15.0;

// 3D Projection Options
GLdouble fovy = 70.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 500;

int scene1Height = 25;
float spikeHeight = 24.9;
bool spikeForward = true;

glm::vec3 cube = glm::vec3(0, 0, 7.5);
glm::vec3 portal1Coords = glm::vec3(0, 0, 0);
glm::vec3 portal2Coords = glm::vec3(0, 0, 0);
bool portal1XNormal = true;
bool portal2XNormal = true;
bool isCubeGrabbed = false;

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
float playerEyeY = 2;
float xangle = 0.0f, yangle = 1.0f, zangle = -1.0f; // Camera angles
float x_pos = 0.0f, y_pos = 0.0f; // Camera yaw
float sensitivity = 0.005f;

int sceneNumber = 2;

// Model Variables
Model_3DS model_house;
Model_3DS model_spike;
Model_3DS model_cube;
Model_3DS model_gun;
Model_3DS model_player;
Model_3DS model_crate;
Model_3DS model_ring;
Model_3DS model_button;

// Textures
GLTexture tex_ground;
GLTexture tex_wall;
GLTexture tex_lava;
GLTexture tex_ceiling;
GLTexture tex_spikecube;
GLTexture tex_portal1;
GLTexture tex_portal2;

float playerX = 0;
float playerY = 0;
float playerZ = 7.5;
bool keystates[256];

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 0.0, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);

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
	glEnable(GL_LIGHTING);

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

void RenderGround2() {
	glDisable(GL_LIGHTING);    // Disable lighting 

	glColor3f(1.0, 1.0, 1.0);    // Set color to white

	glEnable(GL_TEXTURE_2D);    // Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);    // Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);    // Set quad normal direction.
	glTexCoord2f(0, 0);
	glVertex3f(-15, 0, -15);
	glTexCoord2f(4, 0);
	glVertex3f(-15, 0, 15);
	glTexCoord2f(4, 4);
	glVertex3f(15, 0, 15);
	glTexCoord2f(0, 4);
	glVertex3f(15, 0, -15);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);    // Enable lighting again for other entities coming through the pipeline.

	glColor3f(1, 1, 1);    // Set material back to white instead of grey used for the ground texture.
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

void RenderGun() {
	float dx = playerX - (playerX - xangle);
	float dz = playerZ - (playerZ - zangle);
	float angle = atan2(dx, dz) * (180.0f / 3.141592);
	if (isFPV) {
		glPushMatrix();
		glTranslatef(playerX, playerY + 1.6, playerZ);
		glRotatef(angle - 190, 0, 1, 0);
		if (isCubeGrabbed) {
			glTranslatef(0.3, 0, -1.2);
		}
		else {
			glTranslatef(0.3, 0, -0.9);
		}
		glScalef(0.5, 0.5, 0.5);
		model_gun.Draw();
		glPopMatrix();
	}
	else {
		glPushMatrix();
		glTranslatef(playerX, playerY + 1, playerZ);
		glRotatef(angle - 190, 0, 1, 0);
		glTranslatef(0.3, 0, -0.6);
		glScalef(0.5, 0.5, 0.5);
		model_gun.Draw();
		glPopMatrix();
	}
}

void RenderCrates() {
	glPushMatrix();
	glTranslatef(14.3, 0, 14.3);
	glScalef(0.25, 0.25, 0.25);
	model_crate.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(14.05, 1.25, 14.05);
	glRotatef(45, 0, 1, 0);
	glScalef(0.25, 0.25, 0.25);
	model_crate.Draw();
	glPopMatrix();
}

float ringAnim = 0;
void RenderRings() {
	glPushMatrix();
	glTranslatef(-12, 1, 10);
	glRotatef(ringAnim, 0,0.5, 0);
	glScalef(0.5, 0.5, 0.5);
	model_ring.Draw();
	glPopMatrix();

	ringAnim += 2;
}

void RenderPlayer() {
	float dx = playerX - (playerX - xangle);
	float dz = playerZ - (playerZ - zangle);
	float angle = atan2(dx, dz) * (180.0f / 3.141592);
	glColor3f(1, 1, 1);
	glPushMatrix();
	glTranslatef(playerX, playerY, playerZ);
	glScalef(1, 1, 1);
	glRotatef(angle, 0, 1, 0);
	model_player.Draw();
	glPopMatrix();
}

void drawSpike() {

	glDisable(GL_LIGHTING);	// Disable lighting 
	glBindTexture(GL_TEXTURE_2D, tex_spikecube.texture[0]);	// Bind the ground texture
	glColor3f(1, 1, 1);	// Dim the ground texture a bit
	
	float reps = 15;
	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-15, spikeHeight, -9);
	glTexCoord2f(reps, 0);
	glVertex3f(-15, spikeHeight, 1);
	glTexCoord2f(reps, reps);
	glVertex3f(15, spikeHeight, 1);
	glTexCoord2f(0, reps);
	glVertex3f(15, spikeHeight, -9);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_spikecube.texture[0]);	// Bind the ground texture
	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(15, 0);
	glVertex3f(-15, spikeHeight, 1);
	glTexCoord2f(15, 15);
	glVertex3f(15, spikeHeight, 1);
	glTexCoord2f(0, 15);
	glVertex3f(15, spikeHeight + scene1Height, 1);
	glTexCoord2f(0, 0);		
	glVertex3f(-15, spikeHeight + scene1Height, 1);
	glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);	// Disable lighting 

	glColor3f(1, 1, 1);
	glPushMatrix();
	glTranslatef(0, spikeHeight,0);
	glRotatef(180, 0, 0, 1);
	model_spike.Draw();
	//glutSolidCone(1, 2, 40, 40);
	glPopMatrix();
}

void RenderSpike() {

	glColor3f(1, 1, 1);
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 15; j++) {
			glPushMatrix();
			glTranslatef(j * 2 - 14, 0, i * 2 - 14);
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
	glVertex3f(-wallHalfLength, 0, -wallHalfWidth);
	glTexCoord2f(4, 0);
	glVertex3f(-wallHalfLength, 0, wallHalfWidth);
	glTexCoord2f(4, 4);
	glVertex3f(-wallHalfLength, scene1Height, wallHalfWidth);
	glTexCoord2f(0, 4);
	glVertex3f(-wallHalfLength, scene1Height, -wallHalfWidth);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(wallHalfLength, 0, -wallHalfWidth);
	glTexCoord2f(4, 0);
	glVertex3f(wallHalfLength, 0, wallHalfWidth);
	glTexCoord2f(4, 4);
	glVertex3f(wallHalfLength, scene1Height, wallHalfWidth);
	glTexCoord2f(0, 4);
	glVertex3f(wallHalfLength, scene1Height, -wallHalfWidth);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-wallHalfLength, 0, wallHalfWidth);
	glTexCoord2f(4, 0);
	glVertex3f(wallHalfLength, 0, wallHalfWidth);
	glTexCoord2f(4, 4);
	glVertex3f(wallHalfLength, scene1Height, wallHalfWidth);
	glTexCoord2f(0, 4);
	glVertex3f(-wallHalfLength, scene1Height, wallHalfWidth);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-wallHalfLength, 0, -wallHalfWidth);
	glTexCoord2f(4, 0);
	glVertex3f(wallHalfLength, 0, -wallHalfWidth);
	glTexCoord2f(4, 4);
	glVertex3f(wallHalfLength, scene1Height, -wallHalfWidth);
	glTexCoord2f(0, 4);
	glVertex3f(-wallHalfLength, scene1Height, -wallHalfWidth);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-wallHalfLength, -5, -5);
	glTexCoord2f(4, 0);
	glVertex3f(-wallHalfLength, 0, -5);
	glTexCoord2f(4, 4);
	glVertex3f(-wallHalfLength, 0, 5);
	glTexCoord2f(0, 4);
	glVertex3f(-wallHalfLength, -5, 5);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(wallHalfLength, -5, -5);
	glTexCoord2f(4, 0);
	glVertex3f(wallHalfLength, 0, -5);
	glTexCoord2f(4, 4);
	glVertex3f(wallHalfLength, 0, 5);
	glTexCoord2f(0, 4);
	glVertex3f(wallHalfLength, -5, 5);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(wallHalfLength, -5, -5);
	glTexCoord2f(4, 0);
	glVertex3f(wallHalfLength, 0, -5);
	glTexCoord2f(4, 4);
	glVertex3f(-wallHalfLength, 0, -5);
	glTexCoord2f(0, 4);
	glVertex3f(-wallHalfLength, -5, -5);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(wallHalfLength, -5, 5);
	glTexCoord2f(4, 0);
	glVertex3f(wallHalfLength, 0, 5);
	glTexCoord2f(4, 4);
	glVertex3f(-wallHalfLength, 0, 5);
	glTexCoord2f(0, 4);
	glVertex3f(-wallHalfLength, -5, 5);
	glEnd();
	glPopMatrix();

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void RenderCube() {
	if (isCubeGrabbed) {
		float cubeOffsetX = 3.0f * xangle;
		float cubeOffsetY = 1.0;
		float cubeOffsetZ = 3.0f * zangle;
		float cubeX = playerX + cubeOffsetX;
		float cubeY = playerY + cubeOffsetY;
		float cubeZ = playerZ + cubeOffsetZ;
		cube = glm::vec3(cubeX, cubeY, cubeZ);
	} else if (cube.y > 0) {
		cube.y -= 0.02 / cube.y;
		if (cube.y < 0) {
			cube.y = 0;
		}
	}

	glColor3f(0.48, 0.48, 0.48);
	glPushMatrix();
	glTranslatef(cube.x,cube.y+0.01,cube.z);
	glScalef(0.6, 0.6, 0.6);
	model_cube.Draw();
	glPopMatrix();
	glColor3f(1, 1, 1);
}

void drawPortal1(float x, float y, float z) {
	if (x == 0 && z == 0) {
		return;
	}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_portal1.texture[0]);	// Bind the ground texture
	float startAngle = 0;
	float endAngle = 3.141592;
	int numSegments = 20;

	if (portal1XNormal) {
	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(x - 1, y - 1.75, z);
	glTexCoord2f(4, 0);
	glVertex3f(x - 1, y + 1.75, z);
	glTexCoord2f(4, 4);
	glVertex3f(x + 1, y + 1.75, z);
	glTexCoord2f(0, 4);
	glVertex3f(x + 1, y - 1.75, z);
	glEnd();
	}
	else {
		glPushMatrix();
		glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);	// Set quad normal direction.
		glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
		glVertex3f(x, y - 1.75, z - 1);
		glTexCoord2f(4, 0);
		glVertex3f(x, y + 1.75, z - 1);
		glTexCoord2f(4, 4);
		glVertex3f(x, y + 1.75, z + 1);
		glTexCoord2f(0, 4);
		glVertex3f(x, y - 1.75, z + 1);
		glEnd();
	}

	glPopMatrix();

	glEnable(GL_LIGHTING);
}

void drawPortal2(float x, float y, float z) {
	if (x == 0 && z == 0) {
		return;
	}
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_portal2.texture[0]);	// Bind the ground texture
	glColor3f(1, 1, 1);	// Dim the ground texture a bit
	float startAngle = 0;
	float endAngle = 3.141592;
	int numSegments = 20;

	if (portal2XNormal) {
		glPushMatrix();
		glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);	// Set quad normal direction.
		glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
		glVertex3f(x - 1, y - 1.75, z);
		glTexCoord2f(4, 0);
		glVertex3f(x - 1, y + 1.75, z);
		glTexCoord2f(4, 4);
		glVertex3f(x + 1, y + 1.75, z);
		glTexCoord2f(0, 4);
		glVertex3f(x + 1, y - 1.75, z);
		glEnd();
	}
	else {
		glPushMatrix();
		glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);	// Set quad normal direction.
		glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
		glVertex3f(x, y - 1.75, z - 1);
		glTexCoord2f(4, 0);
		glVertex3f(x, y + 1.75, z - 1);
		glTexCoord2f(4, 4);
		glVertex3f(x, y + 1.75, z + 1);
		glTexCoord2f(0, 4);
		glVertex3f(x, y - 1.75, z + 1);
		glEnd();
	}

	glPopMatrix();

	glEnable(GL_LIGHTING);
}

void setPortal1() {
	glm::vec3 rayDirection = glm::normalize(glm::vec3(xangle, -yangle, zangle)); // Get normalized ray direction
	glm::vec3 rayOrigin;
	bool towardsXWall;
	bool towardsZWall;
	if (isFPV) {
		rayOrigin = glm::vec3(playerX, playerEyeY, playerZ);
		towardsXWall = playerX < playerX + xangle;
		towardsZWall = playerZ < playerZ + zangle;
	}
	else {
		float cameraOffsetX = -6.0f * xangle;
		float cameraOffsetY =  yangle;
		float cameraOffsetZ = -6.0f * zangle;

		float cameraX = playerX + cameraOffsetX;
		float cameraY = playerEyeY + cameraOffsetY;
		float cameraZ = playerZ + cameraOffsetZ;
		rayOrigin = glm::vec3(cameraX, cameraY, cameraZ); // Camera's position is the ray's origin
		rayDirection = glm::normalize(glm::vec3(cameraOffsetX, yangle - 0.4, cameraOffsetZ));
		towardsXWall = cameraX < playerX;
		towardsZWall = cameraZ < playerZ;
	}

	float newX = 0, newY = 0, newZ = 0;
	bool newPortalNormal = portal1XNormal;

	float t = (-15.0f - rayOrigin.x) / rayDirection.x;

	float zAtXn15 = rayOrigin.z + t * rayDirection.z;
	float yAtXn15 = rayOrigin.y + t * rayDirection.y;

	if (!towardsXWall && zAtXn15 <= 15 && zAtXn15 >= -15 && yAtXn15 > 0 && yAtXn15 < scene1Height) {
		newPortalNormal = false;
		newX = -14.99;
		newY = yAtXn15 < 1.75 ? 1.75 : yAtXn15;
		newZ = zAtXn15 < -14 ? -14 : zAtXn15 > 14 ? 14 : zAtXn15;
	}

	t = (15.0f - rayOrigin.x) / rayDirection.x;
	float zAtX15 = rayOrigin.z + t * rayDirection.z;
	float yAtX15 = rayOrigin.y + t * rayDirection.y;
	if (towardsXWall && zAtX15 <= 15 && zAtX15 >= -15 && yAtX15 > 0 && yAtX15 < scene1Height) {
		newPortalNormal = false;
		newX = 14.99;
		newY = yAtX15 < 1.75 ? 1.75 : yAtX15;
		newZ = zAtX15 < -14 ? -14 : zAtX15 > 14 ? 14 : zAtX15;
	}

	t = (15.0f - rayOrigin.z) / rayDirection.z;
	float xAtZ15 = rayOrigin.x + t * rayDirection.x;
	float yAtZ15 = rayOrigin.y + t * rayDirection.y;
	if (towardsZWall && xAtZ15 <= 15 && xAtZ15 >= -15 && yAtZ15 > 0 && yAtZ15 < scene1Height) {
		newPortalNormal = true;
		newX = xAtZ15 < -14 ? -14 : xAtZ15 > 14 ? 14 : xAtZ15;
		newY = yAtZ15 < 1.75 ? 1.75 : yAtZ15;
		newZ = 14.99;
	}

	t = (-15.0f - rayOrigin.z) / rayDirection.z;
	float xAtZn15 = rayOrigin.x + t * rayDirection.x;
	float yAtZn15 = rayOrigin.y + t * rayDirection.y;
	if (!towardsZWall && xAtZn15 <= 15 && xAtZn15 >= -15 && yAtZn15 > 0 && yAtZn15 < scene1Height) {
		newPortalNormal = true;
		newX = xAtZn15 < -14 ? -14 : xAtZn15 > 14 ? 14 : xAtZn15;
		newY = yAtZn15 < 1.75 ? 1.75 : yAtZn15;
		newZ = -14.99;
	}	

	// diagonal = 2.02
	float distance = sqrt(pow(portal2Coords.x - newX, 2) + pow(portal2Coords.y - newY, 2) + pow(portal2Coords.z - newZ, 2));
	if (distance >= 4.04) {
		portal1XNormal = newPortalNormal;
		portal1Coords = glm::vec3(newX, newY, newZ);
	}
	else {
		//play error sound
	}
}

void setPortal2() {
	glm::vec3 rayDirection = glm::normalize(glm::vec3(xangle, -yangle, zangle)); // Get normalized ray direction
	glm::vec3 rayOrigin;
	bool towardsXWall;
	bool towardsZWall;
	if (isFPV) {
		rayOrigin = glm::vec3(playerX, playerEyeY, playerZ);
		towardsXWall = playerX < playerX + xangle;
		towardsZWall = playerZ < playerZ + zangle;
	}
	else {
		float cameraOffsetX = -6.0f * xangle;
		float cameraOffsetY = yangle;
		float cameraOffsetZ = -6.0f * zangle;

		float cameraX = playerX + cameraOffsetX;
		float cameraY = playerEyeY + cameraOffsetY;
		float cameraZ = playerZ + cameraOffsetZ;
		rayOrigin = glm::vec3(cameraX, cameraY, cameraZ); // Camera's position is the ray's origin
		rayDirection = glm::normalize(glm::vec3(cameraOffsetX,yangle - 0.4, cameraOffsetZ));
		towardsXWall = cameraX < playerX;
		towardsZWall = cameraZ < playerZ;
	}

	float newX = 0, newY = 0, newZ = 0;
	bool newPortalNormal = portal2XNormal;

	float t = (-15.0f - rayOrigin.x) / rayDirection.x;

	float zAtXn15 = rayOrigin.z + t * rayDirection.z;
	float yAtXn15 = rayOrigin.y + t * rayDirection.y;

	if (!towardsXWall && zAtXn15 <= 15 && zAtXn15 >= -15 && yAtXn15 > 0 && yAtXn15 < scene1Height) {
		newPortalNormal = false;
		newX = -14.99;
		newY = yAtXn15 < 1.75 ? 1.75 : yAtXn15;
		newZ = zAtXn15 < -14 ? -14 : zAtXn15 > 14 ? 14 : zAtXn15;
	}

	t = (15.0f - rayOrigin.x) / rayDirection.x;
	float zAtX15 = rayOrigin.z + t * rayDirection.z;
	float yAtX15 = rayOrigin.y + t * rayDirection.y;
	if (towardsXWall && zAtX15 <= 15 && zAtX15 >= -15 && yAtX15 > 0 && yAtX15 < scene1Height) {
		newPortalNormal = false;
		newX = 14.99;
		newY = yAtX15 < 1.75 ? 1.75 : yAtX15;
		newZ = zAtX15 < -14 ? -14 : zAtX15 > 14 ? 14 : zAtX15;
	}

	t = (15.0f - rayOrigin.z) / rayDirection.z;
	float xAtZ15 = rayOrigin.x + t * rayDirection.x;
	float yAtZ15 = rayOrigin.y + t * rayDirection.y;
	if (towardsZWall && xAtZ15 <= 15 && xAtZ15 >= -15 && yAtZ15 > 0 && yAtZ15 < scene1Height) {
		newPortalNormal = true;
		newX = xAtZ15 < -14 ? -14 : xAtZ15 > 14 ? 14 : xAtZ15;
		newY = yAtZ15 < 1.75 ? 1.75 : yAtZ15;
		newZ = 14.99;
	}

	t = (-15.0f - rayOrigin.z) / rayDirection.z;
	float xAtZn15 = rayOrigin.x + t * rayDirection.x;
	float yAtZn15 = rayOrigin.y + t * rayDirection.y;
	if (!towardsZWall && xAtZn15 <= 15 && xAtZn15 >= -15 && yAtZn15 > 0 && yAtZn15 < scene1Height) {
		newPortalNormal = true;
		newX = xAtZn15 < -14 ? -14 : xAtZn15 > 14 ? 14 : xAtZn15;
		newY = yAtZn15 < 1.75 ? 1.75 : yAtZn15;
		newZ = -14.99;
	}

	float distance = sqrt(pow(portal1Coords.x - newX, 2) + pow(portal1Coords.y - newY, 2) + pow(portal1Coords.z - newZ, 2));
	if (distance >= 4.04) {
		portal2XNormal = newPortalNormal;
		portal2Coords = glm::vec3(newX, newY, newZ);
	}
	else {
		//play error sound
	}
}

void drawWireCuboid(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Set to draw wireframe

	glColor3f(0.0, 0.0, 1.0); // Set color to white

	glBegin(GL_QUADS);

	// Bottom face
	glVertex3f(minX, minY, minZ);
	glVertex3f(maxX, minY, minZ);
	glVertex3f(maxX, minY, maxZ);
	glVertex3f(minX, minY, maxZ);

	// Top face
	glVertex3f(minX, maxY, minZ);
	glVertex3f(maxX, maxY, minZ);
	glVertex3f(maxX, maxY, maxZ);
	glVertex3f(minX, maxY, maxZ);

	// Front face
	glVertex3f(minX, minY, minZ);
	glVertex3f(maxX, minY, minZ);
	glVertex3f(maxX, maxY, minZ);
	glVertex3f(minX, maxY, minZ);

	// Back face
	glVertex3f(minX, minY, maxZ);
	glVertex3f(maxX, minY, maxZ);
	glVertex3f(maxX, maxY, maxZ);
	glVertex3f(minX, maxY, maxZ);

	// Left face
	glVertex3f(minX, minY, minZ);
	glVertex3f(minX, minY, maxZ);
	glVertex3f(minX, maxY, maxZ);
	glVertex3f(minX, maxY, minZ);

	// Right face
	glVertex3f(maxX, minY, minZ);
	glVertex3f(maxX, minY, maxZ);
	glVertex3f(maxX, maxY, maxZ);
	glVertex3f(maxX, maxY, minZ);

	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Set back to fill mode
}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 100);
}

void grabCube() {
	if (isCubeGrabbed) {
		isCubeGrabbed = false;
		return;
	}
	float cubeDistance = sqrt(pow(cube.x - playerX, 2) + pow(cube.z - playerZ, 2));
	
	if (cubeDistance < 4) {
		isCubeGrabbed = true;
	}
	else {
		// play error sound
	}
}

void drawCrosshair(float x, float y, float z, float r) {
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(x, y, z);
	glutSolidSphere(r, 20, 20);
	glPopMatrix();
	glColor3f(1, 1, 1);
}
//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//setupCamera();
	InitLightSource();
	glLoadIdentity();


	if (isFPV) {
		gluLookAt(playerX, playerEyeY, playerZ, playerX + xangle, playerEyeY - yangle, playerZ + zangle, 0.0f, abs(playerEyeY), 0.0f);
		drawCrosshair(playerX + xangle, playerEyeY - yangle, playerZ + zangle, 0.005);
	}
	else {
		float cameraOffsetX = -6.0f * xangle;
		float cameraOffsetY = yangle;
		float cameraOffsetZ = -6.0f * zangle;

		float cameraX = playerX + cameraOffsetX;
		float cameraY = playerEyeY + cameraOffsetY;
		float cameraZ = playerZ + cameraOffsetZ;
		gluLookAt(cameraX, cameraY, cameraZ, playerX, playerEyeY + 0.4, playerZ, 0.0f, abs(playerEyeY), 0.0f);
		drawCrosshair(playerX, playerEyeY + 0.4, playerZ, 0.01);
		RenderPlayer();
	}

	glEnable(GL_LIGHTING);

	RenderRings();

	RenderCrates();

	RenderCube();

	RenderGun();

	drawPortal1(portal1Coords.x, portal1Coords.y, portal1Coords.z);
	drawPortal2(portal2Coords.x, portal2Coords.y, portal2Coords.z);


	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	//model_house.Draw();
	glPushMatrix();
	glScalef(3, 3, 3);
	glPopMatrix();
	// Draw Ground
	RenderGround();

	RenderWall();

	RenderCeiling();

	RenderSpike();

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

void myDisplay2(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//setupCamera();
	InitLightSource();
	glLoadIdentity();


	if (isFPV) {
		gluLookAt(playerX, playerEyeY, playerZ, playerX + xangle, playerEyeY - yangle, playerZ + zangle, 0.0f, abs(playerEyeY), 0.0f);
		drawCrosshair(playerX + xangle, playerEyeY - yangle, playerZ + zangle, 0.005);
	}
	else {
		float cameraOffsetX = -6.0f * xangle;
		float cameraOffsetY = yangle;
		float cameraOffsetZ = -6.0f * zangle;

		float cameraX = playerX + cameraOffsetX;
		float cameraY = playerEyeY + cameraOffsetY;
		float cameraZ = playerZ + cameraOffsetZ;
		gluLookAt(cameraX, cameraY, cameraZ, playerX, playerEyeY + 0.4, playerZ, 0.0f, abs(playerEyeY), 0.0f);
		drawCrosshair(playerX, playerEyeY + 0.4, playerZ, 0.01);
		RenderPlayer();
	}

	RenderGun();

	// Draw Ground
	RenderGround2();

	RenderWall();

	RenderCeiling();

	drawPortal1(portal1Coords.x, portal1Coords.y, portal1Coords.z);
	drawPortal2(portal2Coords.x, portal2Coords.y, portal2Coords.z);

	glPushMatrix();
	glTranslatef(9.0, 0.0, 9.0);  // Adjust the translation based on your model size and desired position
	model_button.Draw();
	glPopMatrix();

	drawWireCuboid(-14.0, 0.0, -14.0, -4.0, scene1Height/1.5, -4.0);

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
	//model_player.Draw();

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
	case 'g':
		grabCube();
		break;
	}
}

void KeyboardUp(unsigned char button, int x, int y) {
	keystates[button] = false;
}

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
	model_cube.Load("Models/cube/cube.3ds");
	model_spike.Load("Models/spike/spike.3ds");
	model_gun.Load("Models/gun/gun.3ds");
	model_player.Load("Models/player/payer.3ds");
	model_crate.Load("Models/crate/crate.3ds");
	model_ring.Load("Models/ring/ring.3ds");
	//model_tree.Load("Models/mytree/tree1.3ds");
	model_button.Load("Models/button/button.3ds");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	tex_wall.Load("Textures/concrete.bmp");
	tex_ceiling.Load("Textures/ceiling.bmp");
	tex_lava.Load("Textures/lava.bmp");
	tex_portal1.Load("Textures/portal1.bmp");
	tex_portal2.Load("Textures/portal2.bmp");
	tex_spikecube.Load("Textures/spikecube.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

void Mouse(int xx, int yy) {
	bool goingUp = false;
	x_pos = (xx - ww / 2) * sensitivity;
	y_pos = ((yy / (hh / 180.0)) - 90) * TO_RADIANS; // 1000

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

bool groundCollided(float x,  float y, float z) {
	if (sceneNumber == 1 && z >= -5 && z <= 5) {
		if (y >= -5) {
			return false;
		}
	} else if (y >= 0) {
		return false;
	}
	return true;
}

void handleTeleports() {
	if (portal1Coords.x == 0 || portal2Coords.x == 0) {
		return;
	}

	float portal1Distance = sqrt(pow(portal1Coords.x - playerX, 2) + pow(portal1Coords.z - playerZ, 2));
	float portal1YDistance = abs(portal1Coords.y - playerY);
	if (portal1Distance <= 0.25 && portal1YDistance < 3.5) {
		playerX = portal2Coords.x;
		playerY = portal2Coords.y;
		playerEyeY = playerY + 2;
		playerZ = portal2Coords.z;

		if (portal2Coords.x <= -14.98) {
			playerX += 1;
		} else if(portal2Coords.x >= 14.98) {
			std::cout << portal2Coords.x;
			playerX -= 1;
		} else if (portal2Coords.z >= 14.98) {
			playerZ -= 1;
		} else if (portal2Coords.z <= -14.98) {
			playerZ += 1;
		}
		return;
	}
	float portal2Distance = sqrt(pow(portal2Coords.x - playerX, 2) + pow(portal2Coords.z - playerZ, 2));
	float portal2YDistance = abs(portal2Coords.y - playerY);
	if (portal2Distance <= 0.25 && portal2YDistance < 3.5) {
		playerX = portal1Coords.x;
		playerY = portal1Coords.y;
		playerEyeY = playerY + 2;
		playerZ = portal1Coords.z;

		if (portal1Coords.x <= -14.98) {
			playerX += 0.4;
		}
		else if (portal1Coords.x >= 14.98) {
			playerX -= 0.4;
		}
		else if (portal1Coords.z >= 14.98) {
			playerZ -= 0.4;
		}
		else if (portal1Coords.z <= -14.98) {
			playerZ += 0.4;
		}
		return;
	}

}

void movementTimerScene1(int value) {
	handleTeleports();

	float speed = 0.1;
	float newX = playerX;
	float newZ = playerZ;

	if (keystates['w']) { // Forward
		newX += xangle * speed;
		newZ += zangle * speed;
	}
	if (keystates['s']) { // Backward
		newX -= xangle * speed;
		newZ -= zangle * speed;
	}
	if (keystates['d']) { // Right
		newX -= zangle * speed;
		newZ += xangle * speed;
	}
	if (keystates['a']) { // Left
		newX += zangle * speed;
		newZ -= xangle * speed;
	}

	if (newX > wallHalfLength - 0.2) {
		newX = wallHalfLength - 0.2;
	}
	else if (newX < -wallHalfLength + 0.2) {
		newX = -wallHalfWidth + 0.2;
	}

	if (newZ > wallHalfWidth - 0.2) {
		newZ = wallHalfWidth - 0.2;
	}
	else if (newZ < -wallHalfWidth + 0.2) {
		newZ = -wallHalfWidth + 0.2;
	}

	if (!groundCollided(newX, playerY, newZ)) {
		playerEyeY -= speed * 2;
		playerY -= speed * 2;
	}

	playerX = newX;
	playerZ = newZ;

	glutPostRedisplay();
	glutTimerFunc(16, movementTimerScene1, 0);
}

void movementTimerScene2(int value) {
	handleTeleports();

	float speed = 0.1;
	float newX = playerX;
	float newZ = playerZ;

	if (keystates['w']) { // Forward
		newX += xangle * speed;
		newZ += zangle * speed;
	}
	if (keystates['s']) { // Backward
		newX -= xangle * speed;
		newZ -= zangle * speed;
	}
	if (keystates['d']) { // Right
		newX -= zangle * speed;
		newZ += xangle * speed;
	}
	if (keystates['a']) { // Left
		newX += zangle * speed;
		newZ -= xangle * speed;
	}

	if (newX > wallHalfLength - 0.2) {
		newX = wallHalfLength - 0.2;
	}
	else if (newX < -wallHalfLength + 0.2) {
		newX = -wallHalfWidth + 0.2;
	}

	if (newZ > wallHalfWidth - 0.2) {
		newZ = wallHalfWidth - 0.2;
	}
	else if (newZ < -wallHalfWidth + 0.2) {
		newZ = -wallHalfWidth + 0.2;
	}

	if (!groundCollided(newX, playerY, newZ)) {
		playerEyeY -= speed * 2;
		playerY -= speed * 2;
	}

	playerX = newX;
	playerZ = newZ;

	glutPostRedisplay();
	glutTimerFunc(16, movementTimerScene2, 0);

}
void spikeTimer(int value) {
	if (spikeHeight <= 4) {
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

void mouseClicks(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		setPortal1();
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		setPortal2();
	}
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(1500, 1000);

	glutInitWindowPosition(0, 0);

	glutCreateWindow(title);


	glutReshapeFunc(resize);
	glutMouseFunc(mouseClicks);
	glutKeyboardFunc(KeyboardDown);
	glutKeyboardUpFunc(KeyboardUp);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutSpecialFunc(Special);

	if (sceneNumber == 1) {
		glutDisplayFunc(myDisplay);
		glutTimerFunc(0, movementTimerScene1, 0);
		glutTimerFunc(0, spikeTimer, 0);
	}
	else if (sceneNumber == 2) {
	glutDisplayFunc(myDisplay2);
		glutTimerFunc(0, movementTimerScene2, 0);
	}

	glutPassiveMotionFunc(Mouse);

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	gluOrtho2D(0.0, 1500, 0.0, 1000);

	glutMainLoop();
}