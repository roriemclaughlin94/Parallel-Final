//Rorie McLaughlin - 40118776


//Following #includes add libraries and other classes to main 
#include <graphics_framework.h>
#include <glm\glm.hpp>
#include <algorithm>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <omp.h>
#include "vec3f.h"
#include "imageloader.h"
#include <thread>
using namespace std;
using namespace graphics_framework;
using namespace glm;



//Following declares all variables for entire project on main.cpp
const float PI = 3.1415926535f;
const float BOX_SIZE = 7.0f; //The length of each side of the cube
const float ALPHA = 0.75f; //The opacity of each face of the cube
float _angle = 30.0f;
const float FLOOR_TEXTURE_SIZE = 15.0f;
float _pos = 0;  //particle position
float slowdown = 2.0;  //particle speed slow
float velocity = 0.0;  //particle initial speed
float zoom = -40.0;  
float pan = 0.0;
float tilt = 0.0;


int loop;
int fall;
const float RADIUS = 4.0f; //The radius of the sphere

bool _highShininess = false; //Whether the shininess parameter is high
bool _lowSpecularity = false; //Whether the specularity parameter is high
bool _emission = false; //Whether the emission parameter is turned on

// angle of rotation for the camera direction
float anglesz = 0.0;
// actual vector representing the camera's direction
float lx = 0.0f, lz = -1.0f;
// XZ position of the camera
float x = 0.0f, z = 5.0f;

//These defines the rain particles size and amount of droplets
#define MAX_PARTICLES 100000
#define WCX		64000               
#define WCY		48000
#define RAIN	0

target_camera cam;


void handleKeypress(unsigned char key, int x, int y) {   //Deals with user input for sphere - changes the lighting on it
	switch (key) {
	case 27: //Escape key
		exit(0);
		break;
	case 's':  //Example, s changes the shiniess of the sphere
		_highShininess = !_highShininess;
		break;
	case 'p':
		_lowSpecularity = !_lowSpecularity;
		break;
	case 'e':
		_emission = !_emission; 
		break;
	case 'f':
		glEnable(GL_FOG);  //f enables the fog and g disables

		break;
	case 'g':
		glDisable(GL_FOG);

	}
}


//Variables for the particles postition, if they exist, how long they last and their fase time
typedef struct {
	// Life
	bool alive;	// is the particle alive?
	float life;	// particle lifespan
	float fade; // decay
	// color
	float red;
	float green;
	float blue;
	// Position/direction
	float xpos;
	float ypos;
	float zpos;
	// Velocity/Direction, only goes down in y dir
	float vel;
	// Gravity
	float gravity;
}particles;

// Paticle System loaded
particles par_sys[MAX_PARTICLES];


// Initialise/Reset Particles - give them their attributes
void initParticles(int i) {
	par_sys[i].alive = true;
	par_sys[i].life = 1.0;
	par_sys[i].fade = float(rand() % 100) / 1000.0f + 0.003f;  //How long for each particle to fade

	par_sys[i].xpos = (float)(rand() % -2100) - 50;  //Starting postition on X axis for rain
	par_sys[i].ypos = 30.0;  //How high it starts 
	par_sys[i].zpos = (float)(rand() % 2100) - 10;   //How close it is to the starting camera point Z

	par_sys[i].red = 0.5;
	par_sys[i].green = 0.5;
	par_sys[i].blue = 1.0;

	par_sys[i].vel = velocity;
	par_sys[i].gravity = -0.9;//Gravity pull set to -0.8

}


//Three perpendicular vectors for a face of the cube and  out indicates the
//direction that the face is from the center of the cube
struct Face {
	Vec3f up;
	Vec3f right;
	Vec3f out;
};

//Represents a cube.
struct Cube {
	Face top;
	Face bottom;
	Face left;
	Face right;
	Face front;
	Face back;
};
GLuint _textureId;
Cube _cube; //The cube


//Rotates the vector by the indicated number of degrees about the specified axis
Vec3f rotate(Vec3f v, Vec3f axis, float degrees) {
	axis = axis.normalize();
	float radians = degrees * PI / 180;
	float s = sin(radians);
	float c = cos(radians);
	return v * c + axis * axis.dot(v) * (1 - c) + v.cross(axis) * s;
}

//Rotates the face by the number of degrees about the specified axis
void rotate(Face &face, Vec3f axis, float degrees) {
	face.up = rotate(face.up, axis, degrees);
	face.right = rotate(face.right, axis, degrees);
	face.out = rotate(face.out, axis, degrees);
}

//Rotates the cube by th number of degrees about the specified axis
void rotate(Cube &cube, Vec3f axis, float degrees) {
	rotate(cube.top, axis, degrees);
	rotate(cube.bottom, axis, degrees);
	rotate(cube.left, axis, degrees);
	rotate(cube.right, axis, degrees);
	rotate(cube.front, axis, degrees);
	rotate(cube.back, axis, degrees);
}



//Initialises the up, right, and out vectors for the six faces of the cube
void initCube(Cube &cube) {


		cube.top.up = Vec3f(0, 0, -1);
		cube.top.right = Vec3f(1, 0, 0);
		cube.top.out = Vec3f(0, 1, 0);

		cube.bottom.up = Vec3f(0, 0, 1);
		cube.bottom.right = Vec3f(1, 0, 0);
		cube.bottom.out = Vec3f(0, -1, 0);

		cube.left.up = Vec3f(0, 0, -1);
		cube.left.right = Vec3f(0, 1, 0);
		cube.left.out = Vec3f(-1, 0, 0);

		cube.right.up = Vec3f(0, -1, 0);
		cube.right.right = Vec3f(0, 0, 1);
		cube.right.out = Vec3f(1, 0, 0);

		cube.front.up = Vec3f(0, 1, 0);
		cube.front.right = Vec3f(1, 0, 0);
		cube.front.out = Vec3f(0, 0, 1);

		cube.back.up = Vec3f(1, 0, 0);
		cube.back.right = Vec3f(0, 1, 0);
		cube.back.out = Vec3f(0, 0, -1);
	}

//Returns if face1 is in back of face2
bool compareFaces(Face* face1, Face* face2) {
	return face1->out[2] < face2->out[2];
}

//Stores the four vertices of the face in the array "vs".
void faceVertices(Face &face, Vec3f* vs) {
	vs[0] = BOX_SIZE / 2 * (face.out - face.right - face.up);
	vs[1] = BOX_SIZE / 2 * (face.out - face.right + face.up);
	vs[2] = BOX_SIZE / 2 * (face.out + face.right + face.up);
	vs[3] = BOX_SIZE / 2 * (face.out + face.right - face.up);
}


//The follow draw face methods draw each face for the cube and gives them colours. This is done to enable alpha blending on specfic face in the order method
void drawTopFace(Face &face) {
	Vec3f vs[4];
	faceVertices(face, vs);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	glColor4f(1.0f, 1.0f, 0.0f, ALPHA);
	glNormal3f(face.out[0], face.out[1], face.out[2]);
	glVertex3f(vs[0][0], vs[0][1], vs[0][2]);
	glVertex3f(vs[1][0], vs[1][1], vs[1][2]);
	glVertex3f(vs[2][0], vs[2][1], vs[2][2]);
	glVertex3f(vs[3][0], vs[3][1], vs[3][2]);

	glEnd();
}

void drawBottomFace(Face &face) {
	Vec3f vs[4];
	faceVertices(face, vs);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	glColor4f(1.0f, 0.0f, 1.0f, ALPHA);
	glNormal3f(face.out[0], face.out[1], face.out[2]);
	glVertex3f(vs[0][0], vs[0][1], vs[0][2]);
	glVertex3f(vs[1][0], vs[1][1], vs[1][2]);
	glVertex3f(vs[2][0], vs[2][1], vs[2][2]);
	glVertex3f(vs[3][0], vs[3][1], vs[3][2]);

	glEnd();
}

void drawLeftFace(Face &face) {
	Vec3f vs[4];
	faceVertices(face, vs);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	glNormal3f(face.out[0], face.out[1], face.out[2]);
	glColor4f(0.0f, 1.0f, 1.0f, ALPHA);
	glVertex3f(vs[0][0], vs[0][1], vs[0][2]);
	glVertex3f(vs[1][0], vs[1][1], vs[1][2]);
	glColor4f(0.0f, 0.0f, 1.0f, ALPHA);
	glVertex3f(vs[2][0], vs[2][1], vs[2][2]);
	glVertex3f(vs[3][0], vs[3][1], vs[3][2]);

	glEnd();
}

void drawRightFace(Face &face) {
	Vec3f vs[4];
	faceVertices(face, vs);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	glNormal3f(face.out[0], face.out[1], face.out[2]);
	glColor4f(1.0f, 0.0f, 0.0f, ALPHA);
	glVertex3f(vs[0][0], vs[0][1], vs[0][2]);
	glVertex3f(vs[1][0], vs[1][1], vs[1][2]);
	glColor4f(0.0f, 1.0f, 0.0f, ALPHA);
	glVertex3f(vs[2][0], vs[2][1], vs[2][2]);
	glVertex3f(vs[3][0], vs[3][1], vs[3][2]);

	glEnd();
}

void drawFrontFace(Face &face, GLuint textureId) {
	Vec3f vs[4];
	faceVertices(face, vs);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glColor4f(1.0f, 1.0f, 1.0f, ALPHA);

	glBegin(GL_QUADS);

	glNormal3f(face.out[0], face.out[1], face.out[2]);
	glTexCoord2f(0, 0);
	glVertex3f(vs[0][0], vs[0][1], vs[0][2]);
	glTexCoord2f(0, 1);
	glVertex3f(vs[1][0], vs[1][1], vs[1][2]);
	glTexCoord2f(1, 1);
	glVertex3f(vs[2][0], vs[2][1], vs[2][2]);
	glTexCoord2f(1, 0);
	glVertex3f(vs[3][0], vs[3][1], vs[3][2]);

	glEnd();
}

void drawBackFace(Face &face, GLuint textureId) {
	Vec3f vs[4];
	faceVertices(face, vs);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glColor4f(1.0f, 1.0f, 1.0f, ALPHA);

	glBegin(GL_QUADS);

	glNormal3f(face.out[0], face.out[1], face.out[2]);
	glTexCoord2f(0, 0);
	glVertex3f(vs[0][0], vs[0][1], vs[0][2]);
	glTexCoord2f(0, 1);
	glVertex3f(vs[1][0], vs[1][1], vs[1][2]);
	glTexCoord2f(1, 1);
	glVertex3f(vs[2][0], vs[2][1], vs[2][2]);
	glTexCoord2f(1, 0);
	glVertex3f(vs[3][0], vs[3][1], vs[3][2]);

	glEnd();
}

//Draws the indicated face on the specified cube
void drawFace(Face* face, Cube &cube, GLuint textureId) {

		if (face == &(cube.top)) {
			drawTopFace(cube.top);
		}
		else if (face == &(cube.bottom)) {
			drawBottomFace(cube.bottom);
		}
		else if (face == &(cube.left)) {
			drawLeftFace(cube.left);
		}
		else if (face == &(cube.right)) {
			drawRightFace(cube.right);
		}
		else if (face == &(cube.front)) {
			drawFrontFace(cube.front, textureId);
		}
		else {
			drawBackFace(cube.back, textureId);
		}
	


}

//Makes the image into a mipmapped texture, and returns the id of the texture
GLuint loadMipmappedTexture(Image *image) {
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	gluBuild2DMipmaps(GL_TEXTURE_2D,
		GL_RGB,
		image->width, image->height,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		image->pixels);
	return textureId;
}

void initRendering() {//Loads in the fog, loops the particles for infinite, start the alpha blending for the cube and loads in the image for mipmapping
	int x, z;

	glShadeModel(GL_SMOOTH);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	
	glClearColor(0.5f, 0.5f, 0.5f, 1);

	
	// Initialize particles
	for (loop = 0; loop < MAX_PARTICLES; loop++) {
		initParticles(loop);
	}
	glEnable(GL_BLEND); //Enable alpha blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Set the blend function
	Image* image = loadBMP("checkerboard.bmp");
	_textureId = loadMipmappedTexture(image);
	delete image;
	
	

}


// For Rain attributes and actions
void drawRain() {
	float x, y, z;
			for (loop = 0; loop < MAX_PARTICLES; loop = loop + 2) {
			if (par_sys[loop].alive == true) {

				x = par_sys[loop].xpos;
				y = par_sys[loop].ypos;
				z = par_sys[loop].zpos + zoom;

				// Draw particles
				glColor3f(0.5, 0.5, 1.0);
				glBegin(GL_LINES);
				glVertex3f(x, y, z);
				glVertex3f(x, y + 0.5, z);
				glEnd();

				// Update values
				//Move
				// Adjust slowdown for speed
				par_sys[loop].ypos += par_sys[loop].vel / (slowdown * 1000);
				par_sys[loop].vel += par_sys[loop].gravity;
				// Decay
				par_sys[loop].life -= par_sys[loop].fade;
				
				{
					if (par_sys[loop].ypos <= -10) {
						par_sys[loop].life = -1.0;
					}
					//Revive 

					if (par_sys[loop].life < 0.0) {
						initParticles(loop);
					}
				}
			}
		}
	}



void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(100.0, (float)w / (float)h, 1.0, 100.0);
}

void drawScene() {

	int i, j;
//	float x, y, z;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Set the camera
	gluLookAt(x, 1.0f, z,
		x + lx, 1.0f, z + lz,
		0.0f, 1.0f, 0.0f);

	glRotatef(pan, 0.0, 1.0, 0.0);
	glRotatef(tilt, 1.0, 0.0, 0.0);

	GLfloat fogColor[] = { 0.5f, 0.5f, 0.5f, 1 };
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, 10.0f);
	glFogf(GL_FOG_END, 25.0f);
	glFogf(GL_FOG_DENSITY, 0.9f);

	glTranslatef(0.0f, 0.0f, -20.0f);

	GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

	GLfloat lightColor[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	GLfloat lightPos[] = { -2 * BOX_SIZE, BOX_SIZE, 4 * BOX_SIZE, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	vector<Face*> faces;
	faces.push_back(&(_cube.top));
	faces.push_back(&(_cube.bottom));
	faces.push_back(&(_cube.left));
	faces.push_back(&(_cube.right));
	faces.push_back(&(_cube.front));
	faces.push_back(&(_cube.back));

	//Sort the faces from back to front

	sort(faces.begin(), faces.end(), compareFaces);
	for (unsigned int i = 0; i < faces.size(); i++) {
		drawFace(faces[i], _cube, _textureId);
	}
	//Draw the floor
	glTranslatef(-10.0f, -15.4f, 0.0f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, _textureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(2000 / FLOOR_TEXTURE_SIZE, _pos / FLOOR_TEXTURE_SIZE);
	glVertex3f(-1000.0f, 0.0f, -1000.0f);
	glTexCoord2f(2000 / FLOOR_TEXTURE_SIZE, (2000 + _pos) / FLOOR_TEXTURE_SIZE);
	glVertex3f(-1000.0f, 0.0f, 1000.0f);
	glTexCoord2f(0.0f, (2000 + _pos) / FLOOR_TEXTURE_SIZE);
	glVertex3f(1000.0f, 0.0f, 1000.0f);
	glTexCoord2f(0.0f, _pos / FLOOR_TEXTURE_SIZE);
	glVertex3f(1000.0f, 0.0f, -1000.0f);

	glEnd();

	glEnd();
	// Which Particles

	if (fall == RAIN) {
		drawRain();
	}
	
	
	//Diffuse (non-shiny) light component
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	//Specular (shiny) light component
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	//Determine the specularity, emissivity, and shininess parameters, based on
	//variables that can be toggled with keystrokes
	float specularity;
	if (_lowSpecularity) {
		specularity = 0.3f;
	}
	else {
		specularity = 1;
	}

	float emissivity;
	if (_emission) {
		emissivity = 0.05f;
	}
	else {
		emissivity = 0;
	}

	float shininess;
	if (_highShininess) {
		shininess = 25;
	}
	else {
		shininess = 12;
	}

	//The color of the sphere
	GLfloat materialColor[] = { 0.2f, 0.2f, 1.0f, 1.0f };
	//The specular (shiny) component of the material
	GLfloat materialSpecular[] = { specularity, specularity, specularity, 1.0f };
	//The color emitted by the material
	GLfloat materialEmission[] = { emissivity, emissivity, emissivity, 1.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
	glMaterialfv(GL_FRONT, GL_EMISSION, materialEmission);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess); //The shininess parameter

	//Draw the sphere
	
	
	glTranslatef(10, 10, 10);
	glutSolidSphere(RADIUS, 350, 150);
	

	glutSwapBuffers();
}

void processSpecialKeys(int key, int xx, int yy) {  //Keys for moving the camera

	float fraction = 0.1f;

	switch (key) {
	case GLUT_KEY_LEFT:
		anglesz -= 0.01f;
		lx = sin(anglesz);
		lz = -cos(anglesz);
		break;
	case GLUT_KEY_RIGHT:
		anglesz += 0.01f;
		lx = sin(anglesz);
		lz = -cos(anglesz);
		break;
	case GLUT_KEY_UP:
		x += lx * fraction;
		z += lz * fraction;
		break;
	case GLUT_KEY_DOWN:
		x -= lx * fraction;
		z -= lz * fraction;
		break;
	}
}

void reshape(int w, int h) {
	if (h == 0) h = 1;

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45, (float)w / (float)h, .1, 200);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void idle() {
	glutPostRedisplay();
}

void update(int value) {     //Updates camrea angle and spins the cube
	rotate(_cube, Vec3f(1, 1, 0), 1);
	_angle += 0.1f;
	if (_angle > 360) {
		_angle -= 360;
	}


	_pos += 0.08f;

	while (_pos > FLOOR_TEXTURE_SIZE) {
		_pos -= FLOOR_TEXTURE_SIZE;
	}

	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}

int main(int argc, char** argv) {     //Main method for loading everything and creating the window of 720p
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1080, 720);

	glutCreateWindow("Coursework part 2");
	initRendering();
	initCube(_cube);

	//Rendering methods
	glutDisplayFunc(drawScene);    
	glutKeyboardFunc(handleKeypress);
	glutReshapeFunc(handleResize);
	glutSpecialFunc(processSpecialKeys);

	glutTimerFunc(25, update, 0);

	glutMainLoop();
	
	return 0;
}




