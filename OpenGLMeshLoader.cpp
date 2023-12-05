#include <iostream>
#include <Windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glut.h>

#pragma comment(lib, "winmm.lib")
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
int score = 2000;
int timer = 0;
bool diamondCollide = false;
bool diamondCollide2 = false;
bool diamondCollide3 = false;
bool buttonCollide = false;
bool frameCollide = false;
float cubePositionX = 0.0f;
int translationDirection = 1;
bool isInsideFrame = false;

void movementTimerScene2(int value);
// 3D Projection Options
GLdouble fovy = 70.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 500;

int scene1Height = 25;
float spikeHeight = 24.9;
float spikeCubeHeight = 3;
bool spikeForward = true;

glm::vec3 cube = glm::vec3(-8, 0, -8);
glm::vec3 ring = glm::vec3(-12, 1, 10);
glm::vec3 portal1Coords = glm::vec3(0, 0, 0);
glm::vec3 portal2Coords = glm::vec3(0, 0, 0);
float portal1Scale = 0;
float portal2Scale = 0;
bool portal1XNormal = true;
bool portal2XNormal = true;
bool isCubeGrabbed = false;
bool isRingCollected = false;
bool isButtonPressed = false;
bool isPlayerOnButton = false;
bool isGameOver = false;
bool isPlayerFrozen = false;


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

int sceneNumber = 1;

// Model Variables
Model_3DS model_house;
Model_3DS model_spike;
Model_3DS model_cube;
Model_3DS model_gun;
Model_3DS model_player;
Model_3DS model_crate;
Model_3DS model_ring;
Model_3DS model_button;
Model_3DS model_fire;
Model_3DS model_diamond;
Model_3DS model_candle;
// Textures
GLTexture tex_ground;
GLTexture tex_ground2;
GLTexture tex_wall;
GLTexture tex_wall2;
GLTexture tex_lava;
GLTexture tex_ceiling;
GLTexture tex_ceiling2;
GLTexture tex_spikecube;
GLTexture tex_portal1;
GLTexture tex_portal2;
GLTexture tex_frame;
GLTexture tex_gameOver;

float playerX = 0;
float playerY = 0;
float playerZ = -10;
float originalY = 0;
bool keystates[256];

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT7);

	// Set light properties
	GLfloat lightPosition[] = { 0, 5, 0.0f, 1.0f };
	GLfloat lightDiffuse[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	if (sceneNumber == 1) {
		lightDiffuse[0] = 0.6;
		lightDiffuse[1] = 0.6;
		lightDiffuse[2] = 0.6;
	}

	glLightfv(GL_LIGHT7, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT7, GL_DIFFUSE, lightDiffuse);

}

void playCollect() {
	PlaySound(TEXT("collect.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void playCollide() {
	PlaySound(TEXT("collide.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void playDead() {
	PlaySound(TEXT("loss.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void playWin() {
	PlaySound(TEXT("win.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void playButtonPressed() {
	PlaySound(TEXT("targetCollect.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void playTeleport() {
	PlaySound(TEXT("teleport.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void resetPlayer() {
	playerX = 0;
	playerY = 0;
	playerZ = -10;
	spikeHeight = 24.9;
	spikeForward = true;
	isCubeGrabbed = false;
	playDead();
	portal1Coords = glm::vec3(0, 0, 0);
	portal2Coords = glm::vec3(0, 0, 0);
	cube = glm::vec3(-8, 0, -8);
	score -= 50;
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

	glColor3f(1.0, 1.0, 1.0);    // Set color to white

	glEnable(GL_TEXTURE_2D);    // Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground2.texture[0]);    // Bind the ground texture

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

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ceiling.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, -1, 0);	// Set quad normal direction.
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

void RenderCeiling2() {

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ceiling2.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, -1, 0);	// Set quad normal direction.
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

void displayGameOver() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Draw "Game Over" message
	glColor3f(1.0f, 1.0f, 1.0f);
	char gameOverText[] = "Game Over";
	glRasterPos2f(WIDTH / 2 - 50, HEIGHT / 2);
	for (size_t i = 0; i < strlen(gameOverText); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, gameOverText[i]);
	}

	// Draw score
	char scoreText[50];
	snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
	glRasterPos2f(WIDTH / 2 - 50, HEIGHT / 2 - 50);
	for (size_t i = 0; i < strlen(scoreText); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreText[i]);
	}

	// Draw timer
	char timerText[50];
	snprintf(timerText, sizeof(timerText), "Timer: %d", timer);
	glRasterPos2f(WIDTH / 2 - 50, HEIGHT / 2 - 100);
	for (int i = 0; timerText[i] != '\0'; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, timerText[i]);
	}

	// Restore the projection and modelview matrices
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_LIGHTING);
}




float gunZAnim = -0.9;
bool gunYForward = false;
float gunYAngle = 0;
void RenderGun() {
	float dx = playerX - (playerX - xangle);
	float dz = playerZ - (playerZ - zangle);
	float angle = atan2(dx, dz) * (180.0f / 3.141592);

	if (!gunYForward) {
		if (gunYAngle > 0) {
			gunYAngle -=3;
		}
	}
	else {
		if (gunYAngle < 33) {
			gunYAngle += 3;
		}
		else {
			gunYForward = false;
		}
	}

	if (isFPV) {
		glPushMatrix();
		glTranslatef(playerX, playerY + 1.6, playerZ);
		glRotatef(angle - 190, 0, 1, 0);
		if (isCubeGrabbed) {
			if (gunZAnim > -1.2) {
				gunZAnim -= 0.05;
			}
			glTranslatef(0.3, 0, gunZAnim);
		}
		else {
			gunZAnim = -0.9;
			glTranslatef(0.3, 0, -0.9);
		}
		glRotatef(gunYAngle, 1, 0, 0);
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
	if (isRingCollected) {
		return;
	}
	glPushMatrix();
	glTranslatef(ring.x,ring.y,ring.z);
	glRotatef(ringAnim, 0,0.5, 0);
	glScalef(0.5, 0.5, 0.5);
	model_ring.Draw();
	glPopMatrix();

	ringAnim += 2;
}

float diamondScale = 0.7;
float scaleIncrement = 0.01; 
float minScale = 0.7;
float maxScale = 1.0;
bool growing = true;  

void RenderDiamond() {
	glPushMatrix();

	glTranslatef(-10, 0.5, -10);
	glRotatef(ringAnim, 0, 0.5, 0);

	if (growing) {
		diamondScale += scaleIncrement;
	}
	else {
		diamondScale -= scaleIncrement;
		
	}

	if (diamondScale >= maxScale || diamondScale <= minScale) {
		growing = !growing;
	}

	glScalef(diamondScale, diamondScale, diamondScale);

	model_diamond.Draw();

	glPopMatrix();

	ringAnim += 2;
}

void RenderDiamond2() {
	glPushMatrix();

	glTranslatef(10, 0.5, -10);
	glRotatef(ringAnim, 0, 0.5, 0);

	if (growing) {
		diamondScale += scaleIncrement;
	}
	else {
		diamondScale -= scaleIncrement;

	}

	if (diamondScale >= maxScale || diamondScale <= minScale) {
		growing = !growing;
	}

	glScalef(diamondScale, diamondScale, diamondScale);

	model_diamond.Draw();

	glPopMatrix();

	ringAnim += 2;
}

void RenderDiamond3() {
	glPushMatrix();

	glTranslatef(-10, 0.5, 10);
	glRotatef(ringAnim, 0, 0.5, 0);

	if (growing) {
		diamondScale += scaleIncrement;
	}
	else {
		diamondScale -= scaleIncrement;

	}

	if (diamondScale >= maxScale || diamondScale <= minScale) {
		growing = !growing;
	}

	glScalef(diamondScale, diamondScale, diamondScale);

	model_diamond.Draw();

	glPopMatrix();

	ringAnim += 2;
}

void diamondCollision() {
	if (diamondCollide) {
		return;
	}
	if (playerX > -11.0 && playerX<-9.3 && playerZ > -11.0 && playerZ < -9.3) {
		if (!diamondCollide) {
			playCollect();
		}
		diamondCollide = true;
		score += 50;
	}
}

void diamondCollision2() {
	if (diamondCollide2) {
		return;
	}
	if (playerX < 11.0 && playerX > 9.3 && playerZ > -11.0 && playerZ < -9.3) {
		if (!diamondCollide2) {
			playCollect();
		}
		diamondCollide2 = true;
		score += 50;
	}
}

void buttonCollision() {
	if (!buttonCollide && playerX > 8.0 && playerX < 10.0 && playerZ > 8.0 && playerZ < 10.0) {
		playerY += 0.4;
		playerEyeY = playerY + 2;
		buttonCollide = true;
	}
	else if (buttonCollide && (playerX < 8.0 || playerX > 10.0 || playerZ < 8.0 || playerZ > 10.0)) {
		buttonCollide = false;
		playerY -= 0.4;
		playerEyeY = playerY + 2;
	}
}

void frameCollision() {
	if(!isInsideFrame)
		{if (playerX <= -3.5) {
				if (playerZ <= -3.5) {
					playerX += 0.1;
					playerZ += 0.1;
				}
		}}
	else {
		if (playerX >= -4.5|| playerZ >= -4.5) {
			playerX -= 0.1;
			playerZ -= 0.1;
			}
		}
	}

void cubeCollision() {
	float cubeDistance = sqrt(pow(cube.x - playerX, 2) + pow(cube.z - playerZ, 2));

	// Check if the player is within the collision distance from any side of the cube
	if (cubeDistance <= 1) {
		// Determine the direction of the collision relative to the cube's position
		float relativeX = playerX - cube.x;
		float relativeZ = playerZ - cube.z;

		// Adjust player position based on the side of the cube
		if (fabs(relativeX) > fabs(relativeZ)) {
			// Adjust along the x-axis
			playerX += (relativeX > 0) ? 0.1 : -0.1;
		}
		else {
			// Adjust along the z-axis
			playerZ += (relativeZ > 0) ? 0.1 : -0.1;
		}
	}
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

float lightTimer = 1 / 60;
void RenderCubeLight() {
	glEnable(GL_LIGHTING);
	float cubePosition[] = { cubePositionX, scene1Height - 0.51, 0.0f };
	if (cubePositionX > 15 || cubePositionX < -15) {
		translationDirection *= -1; 
	}
	cubePositionX += 0.1f * translationDirection; 

	glLineWidth(6);
	glColor3f(0, 0, 1);
	glPushMatrix();
	glTranslatef(cubePosition[0], cubePosition[1], cubePosition[2]);
	glutWireCube(1.0f);
	glPopMatrix();

	GLfloat lightPosition[] = { cubePosition[0], 20, cubePosition[2] - 1, 1.0}; // Set candleTipX, Y, Z to the tip position
	GLfloat lightDiffuse[] = { 0.7, 0.7, 1.0, 1.0 }; // Orange color for light

	glLightfv(GL_LIGHT5, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT5, GL_DIFFUSE, lightDiffuse);
	glLightf(GL_LIGHT5, GL_CONSTANT_ATTENUATION, 0.1);
	glLightf(GL_LIGHT5, GL_LINEAR_ATTENUATION, 0.01);
	glLightf(GL_LIGHT5, GL_QUADRATIC_ATTENUATION, 0.001);
	glEnable(GL_LIGHT5);

	glColor3f(1, 1, 1);
	
}

void drawCandle(float x, float y, float z) {
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(x, y, z);
	glScalef(0.5, 0.5, 0.5);
	model_candle.Draw();
	glPopMatrix();

	lightTimer += 1.0 / 60.0;

	GLfloat lightPosition[] = { x, y + 0.77, z - 1, 1.0 }; // Set candleTipX, Y, Z to the tip position
	GLfloat lightDiffuse[] = { 1.0, 0.5, 0.0, 1.0 }; // Orange color for light
	GLfloat baseIntensity = 0.7; // Base light intensity
	GLfloat amplitude = 0.5; // Amplitude of light intensity variation
	GLfloat currentTime = lightTimer; 
	GLfloat frequency = 2.0; 
	GLfloat lightIntensity = baseIntensity + amplitude * sin(currentTime * frequency);

	glLightfv(GL_LIGHT4, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT4, GL_DIFFUSE, lightDiffuse);
	glLightf(GL_LIGHT4, GL_CONSTANT_ATTENUATION, 0.5);
	glLightf(GL_LIGHT4, GL_LINEAR_ATTENUATION, lightIntensity * 2);
	glLightf(GL_LIGHT4, GL_QUADRATIC_ATTENUATION, lightIntensity * 1);
	glEnable(GL_LIGHT4);

	GLfloat lightPosition1[] = { x + 0.1, y + 0.77, z, 1.0 }; // Set candleTipX, Y, Z to the tip position
	baseIntensity = 0.7; // Base light intensity
	amplitude = 0.5; // Amplitude of light intensity variation
	currentTime = lightTimer;
	frequency = 2.0;
	lightIntensity = baseIntensity + amplitude * sin(currentTime * frequency);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.05);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, lightIntensity * 2);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, lightIntensity * 1);
	glEnable(GL_LIGHT1);

	GLfloat lightPosition2[] = { x - 0.1, y + 0.77, z, 1.0 }; // Set candleTipX, Y, Z to the tip position
	baseIntensity = 0.7; // Base light intensity
	amplitude = 0.5; // Amplitude of light intensity variation
	currentTime = lightTimer;
	frequency = 2.0;
	lightIntensity = baseIntensity + amplitude * sin(currentTime * frequency);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, lightDiffuse);
	glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 0.05);
	glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, lightIntensity * 2);
	glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, lightIntensity * 1);
	glEnable(GL_LIGHT2);

	GLfloat lightPosition3[] = { x, y + 0.77, z + 1, 1.0 }; // Set candleTipX, Y, Z to the tip position
	baseIntensity = 0.7; // Base light intensity
	amplitude = 0.5; // Amplitude of light intensity variation
	currentTime = lightTimer;
	frequency = 2.0;
	lightIntensity = baseIntensity + amplitude * sin(currentTime * frequency);

	glLightfv(GL_LIGHT3, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, lightDiffuse);
	glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 0.05);
	glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, lightIntensity * 2);
	glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, lightIntensity * 1);
	glEnable(GL_LIGHT3);
}

void drawSpike() {
	glColor3f(1, 1, 1);
	glPushMatrix();
	glTranslatef(0, spikeHeight,0);
	glRotatef(180, 0, 0, 1);
	model_spike.Draw();
	glPopMatrix();
}

void RenderSpike() {

	glDisable(GL_LIGHTING);	// Disable lighting 
	glBindTexture(GL_TEXTURE_2D, tex_spikecube.texture[0]);	// Bind the ground texture
	glColor3f(1, 1, 1);	// Dim the ground texture a bit

	float reps = 15;
	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-15, spikeHeight, -15);
	glTexCoord2f(reps, 0);
	glVertex3f(-15, spikeHeight, -5);
	glTexCoord2f(reps, reps);
	glVertex3f(15, spikeHeight, -5);
	glTexCoord2f(0, reps);
	glVertex3f(15, spikeHeight, -15);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-15, spikeHeight + spikeCubeHeight, -15);
	glTexCoord2f(reps, 0);
	glVertex3f(-15, spikeHeight + spikeCubeHeight, -5);
	glTexCoord2f(reps, reps);
	glVertex3f(15, spikeHeight + spikeCubeHeight, -5);
	glTexCoord2f(0, reps);
	glVertex3f(15, spikeHeight + spikeCubeHeight, -15);
	glEnd();
	glPopMatrix();
	
	glBindTexture(GL_TEXTURE_2D, tex_spikecube.texture[0]);	// Bind the ground texture
	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, reps);
	glVertex3f(15, spikeHeight + spikeCubeHeight, -5);
	glTexCoord2f(0, 0);
	glVertex3f(-15, spikeHeight + spikeCubeHeight, -5);
	glTexCoord2f(reps, 0);
	glVertex3f(-15, spikeHeight, -5);
	glTexCoord2f(reps, reps);
	glVertex3f(15, spikeHeight, -5);
	glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);	// Disable lighting 

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
	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_wall.texture[0]);	
	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(1, 0, 0);	// Set quad normal direction.
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
	glNormal3f(-1, 0, 0);	// Set quad normal direction.
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
	glNormal3f(0, 0, -1);	// Set quad normal direction.
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
	glNormal3f(0, 0, 1);	// Set quad normal direction.
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
	glNormal3f(0, 0, 1);	// Set quad normal direction.
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
	glNormal3f(-1, 0, 0);	// Set quad normal direction.
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
	glNormal3f(0, 0, 1);	// Set quad normal direction.
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
	glNormal3f(0, 0, -1);	// Set quad normal direction.
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

void RenderWall2() {
	//glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_wall2.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(1, 0, 0);	// Set quad normal direction.
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
	glNormal3f(-1, 0, 0);	// Set quad normal direction.
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
	glNormal3f(0, 0, -1);	// Set quad normal direction.
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
	glNormal3f(0, 0, 1);	// Set quad normal direction.
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

float cubeAnim = 0;
void RenderCube() {
	isButtonPressed = false;
	if (isCubeGrabbed) {
		float cubeOffsetX = 3.0f * xangle;
		float cubeOffsetY = 1.0;
		float cubeOffsetZ = 3.0f * zangle;
		float cubeX = playerX + cubeOffsetX;
		float cubeY = playerY + cubeOffsetY;
		float cubeZ = playerZ + cubeOffsetZ;
		if (cubeAnim < 1) {
			cubeX = ((cubeX - cube.x) * cubeAnim) + cube.x;
			cubeY = ((cubeY - cube.y) * cubeAnim) + cube.y;
			cubeZ = ((cubeZ - cube.z) * cubeAnim) + cube.z;
			cubeAnim += 0.1;
		}
		cube = glm::vec3(cubeX, cubeY, cubeZ);
	}
	else if (cube.x > 8 && cube.x < 10 && cube.z > 8 && cube.z < 10 && cube.y >= 0.441) {
		cube.y -= 0.02 / cube.y;
		if (cube.y < 0.441) {
			isButtonPressed = true;
			cube.y = 0.441;
		}
	}
	else if (cube.y > 0) {
		cube.y -= 0.02 / cube.y;
		if (cube.y < 0) {
			cube.y = 0;
		}
	}

	if (!isCubeGrabbed) {
		cubeAnim = 0;
	}

	glColor3f(0.48, 0.48, 0.48);
	glPushMatrix();
	glTranslatef(cube.x,cube.y+0.01,cube.z);
	glScalef(0.6, 0.6, 0.6);
	model_cube.Draw();
	glPopMatrix();
	glColor3f(1, 1, 1);
}

bool atTarget2() {
	return playerZ <= -14.4 && playerX < 6 && playerX > 4;
}

void RenderTargetPortal() {
	float x = 0, y = 24.99, z = -14;

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_portal1.texture[0]);	
	glColor3f(0, 1, 0);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glTexCoord2f(0, 0);		
	glVertex3f(x - 1, y, z - 1);
	glTexCoord2f(4, 0);
	glVertex3f(x - 1, y, z + 1);
	glTexCoord2f(4, 4);
	glVertex3f(x + 1, y, z + 1);
	glTexCoord2f(0, 4);
	glVertex3f(x + 1, y, z - 1);
	glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void RenderTargetPortal2() {
	float x = 5, y = 1.75, z = -14.99;

	glEnable(GL_TEXTURE_2D);
	if (isButtonPressed || buttonCollide) {
	glBindTexture(GL_TEXTURE_2D, tex_portal1.texture[0]);
		glColor3f(0, 1, 0);
	} else {
		glBindTexture(GL_TEXTURE_2D, tex_portal2.texture[0]);
		glColor3f(1, 0, 0);
	}

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glTexCoord2f(0, 0);
	glVertex3f(x - 1, y - 1.75, z);
	glTexCoord2f(4, 0);
	glVertex3f(x - 1, y + 1.75, z);
	glTexCoord2f(4, 4);
	glVertex3f(x + 1, y + 1.75, z);
	glTexCoord2f(0, 4);
	glVertex3f(x + 1, y - 1.75, z);
	glEnd();
	glPopMatrix();
}

void drawPortal1(float x, float y, float z) {
	if ((x > -15 && x < -4 && z > -15 && z <-4 && y>scene1Height / 1.5) || (playerX <-4 && playerZ < -4 ) || (x > -2.5 || z > -3.5)) {
		if (x == 0 && z == 0) {
			return;
		}
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_portal1.texture[0]);	// Bind the ground texture
		float startAngle = 0;
		float endAngle = 3.141592;
		int numSegments = 20;

		glPushMatrix();
		glTranslatef(x, y, z);
		glScalef(portal1Scale, portal1Scale, portal1Scale);
		glTranslatef(-x,-y,-z);
		if (portal1XNormal) {
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
		if (portal1Scale < 1) {
			portal1Scale += 0.05;
		}
	}
}

void drawPortal2(float x, float y, float z) {
	if ((x > -15 && x < -4 && z > -15 && z <-4 && y>scene1Height / 1.5) || (playerX < -4 && playerZ < -4) || (x > -2.5 || z > -3.5)) {

		if (x == 0 && z == 0) {
			return;
		}
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_portal2.texture[0]);	// Bind the ground texture
		glColor3f(1, 1, 1);	// Dim the ground texture a bit

		glPushMatrix();
		glTranslatef(x, y, z);
		glScalef(portal2Scale, portal2Scale, portal2Scale);
		glTranslatef(-x, -y, -z);
		if (portal2XNormal) {
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
		if (portal2Scale < 1) {
			portal2Scale += 0.05;
		}
	}
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
		portal1Scale = 0;
		gunYForward = true;
	}
	else {
		playCollide();
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
		portal2Scale = 0;
		gunYForward = true;
	}
	else {
		playCollide();
	}
}

void drawWireCuboid(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, float lineWidth) {
	glDisable(GL_LIGHTING);	// Disable lighting 

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_frame.texture[0]);

	glLineWidth(lineWidth);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Set to draw wireframe

	glColor3f(1.0, 1.0, 1.0); // Set color to white

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

	glDisable(GL_TEXTURE_2D);

	glLineWidth(1.0f);

}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 100);
}

void RenderFire() {
	bool isDead = false;
	float fireRadius = 0.5;

	for (int i = 0; i < 6; i++) {
		glPushMatrix();
		glTranslatef((i / 2.0) + 8, 0, 7.4);
		glScalef(0.5, 0.5, 0.5);
		model_fire.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(7.4, 0, (i / 2.0) + 7.75);
		glScalef(0.5, 0.5, 0.5);
		model_fire.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef((i / 2.0) + 8, 0, 10.6);
		glScalef(0.5, 0.5, 0.5);
		model_fire.Draw();
		glPopMatrix();

		float d1 = sqrt(pow((i / 2.0) + 8 - playerX, 2) + pow(7.4 - playerZ, 2));
		float d2 = sqrt(pow(7.4 - playerX, 2) + pow((i / 2.0) + 7.75 - playerZ, 2));
		float d3 = sqrt(pow((i / 2.0) - playerX, 2) + pow(10.6 - playerZ, 2));

		if (d1 < fireRadius || d2 < fireRadius || d3 < fireRadius) {
			std::cout << "wtf1";
			resetPlayer();
		}
	}

	glPushMatrix();
	glTranslatef(10.5, 0, 10.1);
	glScalef(0.5, 0.5, 0.5);
	model_fire.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(10.5, 0, 7.9);
	glScalef(0.5, 0.5, 0.5);
	model_fire.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(8, 0, 10.1);
	glScalef(0.5, 0.5, 0.5);
	model_fire.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(8, 0, 7.9);
	glScalef(0.5, 0.5, 0.5);
	model_fire.Draw();
	glPopMatrix();

	float d1 = sqrt(pow(10.5 - playerX, 2) + pow(10.1 - playerZ, 2));
	float d2 = sqrt(pow(10.5 - playerX, 2) + pow(7.9 - playerZ, 2));
	float d3 = sqrt(pow(8 - playerX, 2) + pow(10.1 - playerZ, 2));
	float d4 = sqrt(pow(10.1 - playerX, 2) + pow(7.9 - playerZ, 2));
	
	if (d1 < fireRadius || d2 < fireRadius || d3 < fireRadius || d4 < fireRadius) {
		std::cout << "wtf";
		resetPlayer();
	}
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
		playCollide();
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

	RenderRings();

	RenderCrates();

	drawCandle(14, 2.5, 14);

	RenderGun();

	drawPortal1(portal1Coords.x, portal1Coords.y, portal1Coords.z);
	drawPortal2(portal2Coords.x, portal2Coords.y, portal2Coords.z);

	RenderTargetPortal();

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

	{
		glDisable(GL_LIGHTING);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, WIDTH, 0, HEIGHT);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// Draw score
		glColor3f(1.0f, 1.0f, 1.0f);
		char scoreText[50];
		snprintf(scoreText, sizeof(scoreText), "Score: %d", score);

		// Adjust the position based on your preference
		int scoreX = WIDTH - 220;
		int scoreY = HEIGHT - 50;
		int timeX = WIDTH - 370;
		int timeY = HEIGHT - 50;

		glRasterPos2f(scoreX, scoreY);
		for (size_t i = 0; i < strlen(scoreText); i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreText[i]);
		}

		glColor3f(1.0f, 1.0f, 1.0f);
		char timerText[50];
		snprintf(timerText, sizeof(timerText), "Timer: %d", timer);
		glRasterPos2f(timeX, timeY);
		for (int i = 0; timerText[i] != '\0'; i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, timerText[i]);
		}

		// Restore the projection and modelview matrices
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_LIGHTING);
	}
	glutSwapBuffers();
}

void myDisplay2(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//setupCamera();
	InitLightSource();
	glLoadIdentity();
	diamondCollision();
	diamondCollision2();

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

	RenderFire();

	RenderGun();

	RenderCube();
	// Draw Ground
	RenderGround2();

	RenderTargetPortal2();

	RenderWall2();

	RenderCeiling2();

	RenderCubeLight();

	if(!diamondCollide)
		RenderDiamond();

	if (!diamondCollide2)
		RenderDiamond2();

	buttonCollision();

	cubeCollision();

	drawPortal1(portal1Coords.x, portal1Coords.y, portal1Coords.z);
	drawPortal2(portal2Coords.x, portal2Coords.y, portal2Coords.z);

	frameCollision();

	glPushMatrix();
	glTranslatef(9.0, 0.01, 9.0);
	model_button.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.1, 0);
	drawWireCuboid(-16.0, 0.0, -16.0, -4.0, scene1Height/1.5, -4.0,20.0);
	glPopMatrix();

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

	{
		glDisable(GL_LIGHTING);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, WIDTH, 0, HEIGHT);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// Draw score
		glColor3f(1.0f, 1.0f, 1.0f);
		char scoreText[50];
		snprintf(scoreText, sizeof(scoreText), "Score: %d", score);

		// Adjust the position based on your preference
		int scoreX = WIDTH - 220;
		int scoreY = HEIGHT - 50;
		int timeX = WIDTH - 370;
		int timeY = HEIGHT - 50;

		glRasterPos2f(scoreX, scoreY);
		for (size_t i = 0; i < strlen(scoreText); i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreText[i]);
		}

		glColor3f(1.0f, 1.0f, 1.0f);
		char timerText[50];
		snprintf(timerText, sizeof(timerText), "Timer: %d", timer);
		glRasterPos2f(timeX, timeY);
		for (int i = 0; timerText[i] != '\0'; i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, timerText[i]);
		}

		// Restore the projection and modelview matrices
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_LIGHTING);
	}
	if (isButtonPressed && atTarget2()) {
		displayGameOver();
		isGameOver = true;
		isPlayerFrozen = true;
	}
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
	model_diamond.Load("Models/diamond/diamond.3ds");
	model_candle.Load("Models/candle/candle.3ds");
	model_fire.Load("Models/fire/fire.3ds");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	tex_ground2.Load("Textures/ground2.bmp");
	tex_wall.Load("Textures/concrete.bmp");
	tex_wall2.Load("Textures/wall2.bmp");
	tex_ceiling.Load("Textures/ceiling.bmp");
	tex_ceiling2.Load("Textures/concrete2.bmp");
	tex_lava.Load("Textures/lava.bmp");
	tex_portal1.Load("Textures/portal1.bmp");
	tex_portal2.Load("Textures/portal2.bmp");
	tex_spikecube.Load("Textures/spikecube.bmp");
	tex_frame.Load("Textures/wood.bmp");
	tex_gameOver.Load("Textures/gameOver.bmp");
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
	}
	else if (sceneNumber == 2 && playerX > 8.0 && playerX < 10.0 && playerZ > 8.0 && playerZ < 10.0) {
		if (y <= 0.51) return true;
	}
	else if (y > 0) {
		return false;
	}
	return true;
}

void handleRingCollection() {
	if (isRingCollected) {
		return;
	}
	float ringDistance = sqrt(pow(ring.x - playerX, 2) + pow(ring.z - playerZ, 2));
	if (ringDistance <= 1) {
		if (!isRingCollected) {
			playCollect();
		}
		isRingCollected = true;
		score += 50;
		// play collect sound
	}

}

void handleTeleports() {
	if (portal1Coords.x == 0 || portal2Coords.x == 0) {
		return;
	}

	float portal1Distance = sqrt(pow(portal1Coords.x - playerX, 2) + pow(portal1Coords.z - playerZ, 2));
	float portal1YDistance = abs(portal1Coords.y - playerY);
	if (portal1Distance <= 0.8 && portal1YDistance < 3.5) {
		playerX = portal2Coords.x;
		playerY = portal2Coords.y;
		playerEyeY = playerY + 2;
		playerZ = portal2Coords.z;
		playTeleport();

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
		if (playerX < -3.5 && playerZ < -3.5) {
			isInsideFrame = true;
		}
		else {
			isInsideFrame = false;
		}
		return;
	}
	float portal2Distance = sqrt(pow(portal2Coords.x - playerX, 2) + pow(portal2Coords.z - playerZ, 2));
	float portal2YDistance = abs(portal2Coords.y - playerY);
	if (portal2Distance <= 0.8 && portal2YDistance < 3.5) {
		playerX = portal1Coords.x;
		playerY = portal1Coords.y;
		playerEyeY = playerY + 2;
		playerZ = portal1Coords.z;
		playTeleport();

		if (portal1Coords.x <= -14.98) {
			playerX += 1;
		}
		else if (portal1Coords.x >= 14.98) {
			playerX -= 1;
		}
		else if (portal1Coords.z >= 14.98) {
			playerZ -= 1;
		}
		else if (portal1Coords.z <= -14.98) {
			playerZ += 1;
		}
		if (playerX < -3.5 && playerZ < -3.5) {
			isInsideFrame = true;
		}
		else {
			isInsideFrame = false;
		}
		return;
	}

}

void toScene2() {
	sceneNumber = 2;
	portal1Coords = glm::vec3(0, 0, 0);
	portal2Coords = glm::vec3(0, 0, 0);
	playerZ = 15;

	playWin();
	glutDisplayFunc(myDisplay2);
	glutPostRedisplay();
	glutTimerFunc(16, movementTimerScene2, 0);
}

void movementTimerScene1(int value) {
	handleTeleports();
	handleRingCollection();

	float speed = 0.1;
	float newX = playerX;
	float newY = playerY;
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
	
	bool isDead = false;
	bool atTarget = false;

	if (!groundCollided(newX, playerY, newZ)) {
		newY -= speed * 2;
	}

	if (newY <= -5) {
		isDead = true;
	}

	// On top of wall
	if (newZ <= -5 && newY < spikeHeight + spikeCubeHeight && newY > spikeHeight + spikeCubeHeight - 0.3) {
		newY = spikeHeight + spikeCubeHeight;
		if (newY >= 23 && newZ < -13 && newX < 1 && newX > -1) { // Check for goal reached
			atTarget = true;
		} else if (newY >= 23.1) {
			isDead = true;
		}
	}

	if (newZ <= -5 && spikeHeight > newY && spikeHeight - newY < 6) { // Squashed
		isDead = true;
	}


	// Crate Collision
	float crateDistance = sqrt(pow(14 - newX, 2) + pow(14 - newZ, 2));

	if (crateDistance < 1.5) {
		if (newX > newZ) {
			newZ = playerZ;
		}
		else {
			newX = playerX;
		}
	}

	playerX = newX;
	playerY = newY;
	playerZ = newZ;
	playerEyeY = playerY + 2;

	if (isDead) {
		resetPlayer();
	}

	if (atTarget) {
		toScene2();
	}
	else {
		glutPostRedisplay();
		glutTimerFunc(16, movementTimerScene1, 0);
	}
}


void movementTimerScene2(int value) {
	if (!isPlayerFrozen) {
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

	if (atTarget2()) {
		if (isButtonPressed) {
			displayGameOver();
			playWin();
		}
		else {
			playCollide();
		}
	}

	playerX = newX;
	playerZ = newZ;

		glutPostRedisplay();
		glutTimerFunc(16, movementTimerScene2, 0);
	}
}

void spikeTimer(int value) {
	float speed = 0.04;
	if (spikeHeight <= 4) {
		spikeForward = false;
	}
	if (spikeHeight >= 24.9) {
		spikeForward = true;
	}
	if (spikeForward) {
		spikeHeight -= speed;
	}
	else {
		spikeHeight += speed;
	}
	glutPostRedisplay();
	glutTimerFunc(16, spikeTimer, 0);
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

void updateTimer(int value) {
	if (isGameOver) {
		return;
	}
	timer++;	
	score -= 2;
	glutTimerFunc(1000, updateTimer, 0);
	glutPostRedisplay();
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowSize(1500, 1000);

	glutInitWindowPosition(0, 0);

	glutCreateWindow(title);


	glutReshapeFunc(resize);
	glutMouseFunc(mouseClicks);
	glutKeyboardFunc(KeyboardDown);
	glutKeyboardUpFunc(KeyboardUp);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutSpecialFunc(Special);
	glutTimerFunc(1000, updateTimer, 0);

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
	//glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	gluOrtho2D(0.0, 1500, 0.0, 1000);

	glutMainLoop();
}