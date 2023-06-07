//////////////////////////////////////////////////////////////////////////////
// solarSystem.cpp                                                          //
//                                                                          //
// User-defined constants:                                                  //
// CELESTIAL_BODIES is the number of celestial bodies in our solar system   //
// includes the sun and the first 8 planets.                                //
//                                                                          //
// User Interactions:                                                       //
// Press the left/right arrow keys to turn the craft.                       //
// Press the up/down arrow keys to move the craft.                          //
// Scroll the mouse wheel up/down to zoom in/out                            //
// Press space to toggle the orbits drawing.                                //
// Press p to pause/unpause the animation of the solar system.              //
// Press r to reset the spacecraft, animation speed & zooming.              //
//////////////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES 

#include <cmath>
#include <string>
#include <cstdlib>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h> 

using namespace std;
constexpr auto CELESTIAL_BODIES = 9; // Number of celestial bodies

// Globals
static bool pause = false;
static bool drawOrbits = true;
static int MSPF = 50; // Milliseconds per frame
static float xOrtho = 130.0, yOrtho = 80.0;
static float n =  5.0, f = 250.0; // near and far
static float fov = 70.0;  // Field of view angle
static float zoomFactor = 1.1f;  // zoom factor when scrolling
static int width, height; // Size of the OpenGL window
static float angle = 0.0; // Angle of the spacecraft
static float xVal = 0, zVal = 90; // Co-ordinates of the spacecraft
static unsigned int spacecraft; // Display lists base index
static int frameCount = 0; // Number of frames
static int FPS = 0; // Frames per second
static long font = (long)GLUT_BITMAP_8_BY_13; // Font selection
static char theStringBuffer[10]; // String buffer

// Routine to draw a bitmap character string
void writeBitmapString(void* font, char* string) {
	for (char* c = string; *c != '\0'; c++)
		glutBitmapCharacter(font, *c);
}
// Routine to convert floating point to char string
void floatToString(char* destStr, int precision, float val) {
	string str = to_string(val); // convert float to string
	copy(str.begin(), str.end(), destStr);
	destStr[precision] = '\0';
}
// Write data to the screen
void writeData(void) {
	glDisable(GL_LIGHTING); // Disable lighting
	glColor3f(1.0, 1.0, 1.0);
	floatToString(theStringBuffer, 4, FPS);
	glRasterPos3f(-129.0, 94.0, -2.0);
	writeBitmapString((void*)font, (char*)"FPS: ");
	writeBitmapString((void*)font, theStringBuffer);
	glEnable(GL_LIGHTING); // Re-enable lighting
}
// celestial class
class celestial {
public:
	celestial();
	celestial(int b, float r, float o, float s,
		unsigned char colorR, unsigned char colorG, unsigned char colorB);
	float getRadius() { return radius; }
	void setDistFromSun(float d) { distFromSun = d; }
	void incAngles();
	void draw();

private:
	int body;
	enum sun_and_8planets {
		SUN, MERCURY, VENUS, EARTH,
		MARS, JUPITER, SATURN, URANUS, NEPTUNE
	};
	float distFromSun, radius;
	float orbitSpeed, spinnSpeed;
	float orbitAngle, spinnAngle;
	unsigned char color[3];
	void drawOrbitRings();
	void drawMoon();
	void drawSaturnRings();
};

// Asteroid default constructor
celestial::celestial() {
	body = -1;
	radius = 0.0; distFromSun = 0.0;
	orbitSpeed = 0.0; spinnSpeed = 0.0;
	orbitAngle = 0.0; spinnAngle = 0.0;
	color[0] = 0; color[1] = 0; color[2] = 0;
}

// Asteroid constructor
celestial::celestial(int b, float r, float o, float s,
	unsigned char colorR, unsigned char colorG, unsigned char colorB) {
	body = b;
	radius = r; distFromSun = 0.0;
	orbitSpeed = o; spinnSpeed = s;
	orbitAngle = 0.0; spinnAngle = 0.0;
	color[0] = colorR; color[1] = colorG; color[2] = colorB;
}

// Function to increment the orbit and spin angles
void celestial::incAngles() {
	orbitAngle += orbitSpeed;
	spinnAngle += spinnSpeed;
	if (orbitAngle > 360.0) orbitAngle -= 360.0;
	if (spinnAngle > 360.0) spinnAngle -= 360.0;
}

void celestial::drawOrbitRings() {
	if (body != SUN) {
		glDisable(GL_LIGHTING);
		glBegin(GL_LINE_LOOP);
		glColor3f(color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f);
		for (int i = 0; i < 360; i++) {
			glVertex3f(cos(i * (float)M_PI / 180) * distFromSun, 0,
				sin(i * (float)M_PI / 180) * distFromSun);
		}
		glEnd();
		glEnable(GL_LIGHTING);
	}
}

void celestial::drawMoon() {
	float matAmbAndDif[] = {0.5, 0.5, 0.5, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif);
	glPushMatrix();
		glRotatef(orbitAngle, 0.0f, 1.0f, 0.0f);
		glTranslatef(distFromSun, 0, 0);
		glRotatef(orbitAngle * 3, 0.0f, 1.0f, 1.0f);
		glTranslatef(radius * 1.3f, 0, 0);
		glRotatef(spinnAngle, 0.0f, 1.0f, 0.0f);
		glutSolidSphere(radius / 5.0, 30, 30);
	glPopMatrix();
	matAmbAndDif[0] = color[0] / 255.0f;
	matAmbAndDif[1] = color[1] / 255.0f;
	matAmbAndDif[2] = color[2] / 255.0f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif);
}

void celestial::drawSaturnRings() {
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glLineWidth(3.0f);
	glRotatef(20.0, 1.0, 0.0, 1.0);
	for (int j = 0; j < 5; j++) {
		glBegin(GL_LINE_LOOP);
		glColor3f(color[0] / 255.0f - 0.2f, 
				  color[1] / 255.0f - 0.2f, 
				  color[2] / 255.0f - 0.2f);
		for (int i = 0; i < 360; i++) {
			glVertex3f(
				cos(i * M_PI / 180.0f) * radius * (1.25f + j * 0.1f), 0.0f,
				sin(i * M_PI / 180.0f) * radius * (1.25f + j * 0.1f)
			);
		}
		glEnd();
	}
	glLineWidth(1.0f);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void celestial::draw() {
	if (radius > 0.0 || body >= 0) {// If celestial body exists
		glPushMatrix();
		// Applying material properties
		float matAmbAndDif[] = 
			{color[0]/255.0f,color[1]/255.0f, color[2]/255.0f, 1.0};
		float matShine[] = {50.0};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShine);
		// Drawing orbits (toggled by the user)
		if (drawOrbits) drawOrbitRings();
		// If earth, add moon
		if (body == EARTH) drawMoon();
		// Applying General Transformation
		glRotatef(orbitAngle, 0.0f, 1.0f, 0.0f); // orbit rotation
		glTranslatef(distFromSun, 0, 0); // translation from sun
		glRotatef(spinnAngle, 0.0f, 1.0f, 0.0f); // spin rotaion
		//If saturn, add its rings
		if (body == SATURN) drawSaturnRings();
		// If sun, apply light source
		if (body == SUN) {
			float lightPos[] = { 0.0, 0.0, 0.0, 1.0 };
			glColor3ubv(color);
			glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
		}
		// The sphere body
		glutSolidSphere(radius, (int)radius * 6, (int)radius * 6);
		glPopMatrix();
	}
}

enum sun_and_8planets { // Enum for solar system
	SUN, MERCURY, VENUS, EARTH,
	MARS, JUPITER, SATURN, URANUS, NEPTUNE
};

celestial solarSystem[CELESTIAL_BODIES]; // array of solar system bodies

// Routine to count the number of frames drawn every second
void frameCounter(int value) {
	FPS = frameCount;
	frameCount = 0;
	if (pause) {
		frameCount--;
		glutPostRedisplay();
	}
	glutTimerFunc(1000, frameCounter, 1);
}

// Animation routine
void animate(int value) {
	for (int i = 0; i < CELESTIAL_BODIES; i++)
		solarSystem[i].incAngles();
	glutPostRedisplay();
	if (!pause) glutTimerFunc(MSPF, animate, 1);
}

// Initialization routine
void setup(void) {
	// Spacecraft display list
	spacecraft = glGenLists(1);
	glNewList(spacecraft, GL_COMPILE);
	glPushMatrix();
		glDisable(GL_LIGHTING);
		glRotatef(180.0, 0.0, 1.0, 0.0);
		glColor3f(1.0, 1.0, 1.0);
		glutWireCone(5.0, 10.0, 10, 10);
		glEnable(GL_LIGHTING);
		glPopMatrix();
	glEndList();
	// Initialize the solar system celestial bodies
	solarSystem[SUN]     = celestial(0, 25.0, 0.00, 0.0, 255, 215,   0);
	solarSystem[MERCURY] = celestial(1,  3.0, 5.00, 5.0, 178, 153, 110);
	solarSystem[VENUS]   = celestial(2,  5.0, 3.75, 5.0, 245, 211, 107);
	solarSystem[EARTH]   = celestial(3,  5.0, 3.00, 5.0,  41, 121, 255);
	solarSystem[MARS]    = celestial(4,  4.0, 2.50, 5.0, 255,  10,  10);
	solarSystem[JUPITER] = celestial(5,  7.0, 1.25, 5.0, 233, 214, 107);
	solarSystem[SATURN]  = celestial(6,  5.0, 1.00, 5.0, 240, 229, 168);
	solarSystem[URANUS]  = celestial(7,  4.0, 0.75, 5.0, 157, 231, 247);
	solarSystem[NEPTUNE] = celestial(8,  3.0, 0.50, 5.0,  90, 120, 196);
	// Setting the distances of the planets from the sun
	float radius_sum = solarSystem[SUN].getRadius() + 10.0f;
	for (int i = 1; i < CELESTIAL_BODIES; i++) {
		radius_sum += 3.0f + solarSystem[i].getRadius();
		solarSystem[i].setDistFromSun(radius_sum);
		radius_sum += solarSystem[i].getRadius();
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glClearColor(0.10f, 0.10f, 0.10f, 0.0f);
	glutTimerFunc(0, animate, 0); // Initial call of animate()
	glutTimerFunc(0, frameCounter, 0); // Initial call of frameCounter()
}

// Drawing routine.
void drawScene(void) {
	frameCount++; // Increment number of frames every redrawing process
	// Lighting stuff
	float lightAmb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	float lightDif[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif);
	glEnable(GL_LIGHT0);
	// Begin spacecraft viewport.
	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);
	glEnable(GL_SCISSOR_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Showing FPS on screen
	writeData();
	// Prespective projection
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	gluPerspective(fov, (float)width / height, n, f);

	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	// Locate the camera at tip of cone and pointing in cone direction
	gluLookAt(xVal - 10 * sin((M_PI / 180.0) * angle), 2.0,
		      zVal - 10 * cos((M_PI / 180.0) * angle),
		      xVal - 11 * sin((M_PI / 180.0) * angle), 2.0,
		      zVal - 11 * cos((M_PI / 180.0) * angle),
		      0.0, 1.0, 0.0);
	glDisable(GL_LIGHTING);
	// Draw all the celestial bodies
	solarSystem[SUN].draw();
	glEnable(GL_LIGHTING);
	for (int i = MERCURY; i < CELESTIAL_BODIES; i++)
		solarSystem[i].draw();
	// Draw spacecraft
	glPushMatrix();
		glTranslatef(xVal, 0.0, zVal);
		glRotatef(angle, 0.0, 1.0, 0.0);
		glCallList(spacecraft);
	glPopMatrix();
	// End spacecraft viewport
	//------------------------------------------------------------------------
	// Begin lower-right viewport
	int x = width * 0.70, y = height * 0.02;
	int w = width * 0.29, h = height * 0.35;
	glViewport(x, y, w, h);
	glScissor(x, y, w, h);
	glEnable(GL_SCISSOR_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float matAmbAndDif[] = { 1.0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif);
	// Orthographic projection
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(-xOrtho, xOrtho, -yOrtho, yOrtho, n, f);

	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	float z = -125.0; // -125.0
	glTranslatef(0, 0, z);
	// Fixed camera
	gluLookAt(0.0, 15.0, 19.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glDisable(GL_LIGHTING);
	// Draw all the celestial bodies
	solarSystem[SUN].draw();
	glEnable(GL_LIGHTING);
	for (int i = MERCURY; i < CELESTIAL_BODIES; i++)
		solarSystem[i].draw();
	// Draw spacecraft
	glPushMatrix();
		glTranslatef(xVal, 0.0, zVal);
		glRotatef(angle, 0.0, 1.0, 0.0);
		glCallList(spacecraft);
	glPopMatrix();
	// End lower-right viewport
	glDisable(GL_SCISSOR_TEST);
	glutSwapBuffers();
}

// OpenGL window reshape routine
void resize(int w, int h) {
	glViewport(0, 0, w, h);
	width = w; height = h; // Pass the size of the OpenGL window
}

// Keyboard input processing routine
void keyInput(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		exit(0);
		break;
	// Reset the values
	case 'r':
		fov = 70.0; // Zooming
		xVal = 0.0; zVal = 90.0; // Camera & spacecraft position
		angle = 0.0; // Camera & spacecraft angle
		MSPF = 50; // Animation speed
		if (pause) glutPostRedisplay();
		break;
	// change the animation speed
	case '+':
		if (MSPF < 100) MSPF += 5;
		break;
	case '-':
		if(MSPF > 5) MSPF -= 5;
		break;
	// toggle orbits drawing
	case ' ':
		drawOrbits = !drawOrbits;
		if (pause) glutPostRedisplay();
		break;
	// pause/unpause animation
	case 'p':
		pause = !pause;
		if (!pause) glutTimerFunc(MSPF, animate, 1);
		break;
	default:
		break;
	}
}

// Callback routine for non-ASCII key entry
void specialKeyInput(int key, int x, int y) {
	float tempxVal = xVal, tempzVal = zVal, tempAngle = angle;
	// Compute next position
	if (key == GLUT_KEY_LEFT)  tempAngle = (float)(angle + 5.0);
	if (key == GLUT_KEY_RIGHT) tempAngle = (float)(angle - 5.0);
	if (key == GLUT_KEY_UP) {
		tempxVal = (float)(xVal - sin(angle * M_PI / 180.0));
		tempzVal = (float)(zVal - cos(angle * M_PI / 180.0));
	}
	if (key == GLUT_KEY_DOWN) {
		tempxVal = (float)(xVal + sin(angle * M_PI / 180.0));
		tempzVal = (float)(zVal + cos(angle * M_PI / 180.0));
	}
	// Angle correction
	if (tempAngle > 360.0) tempAngle -= 360.0;
	if (tempAngle < 0.0) tempAngle += 360.0;
	// Move spacecraft to next position
	xVal = tempxVal;
	zVal = tempzVal;
	angle = tempAngle;
	glutPostRedisplay();
}

// Mouse wheel callback routine that handles zooming in/out
void mouse(int button, int state, int x, int y) {
	// scroll up -> zoom in
	if (button == 3 && fov > 25.0)
		fov /= zoomFactor;
	// scroll down -> zoom out
	else if (button == 4 && fov < 100)
		fov *= zoomFactor;
	resize(width, height);
	glutPostRedisplay();
}

// Routine to output interaction instructions to the C++ window
void printInteraction(void) {
	cout << "Interaction:" << endl;
	cout << "Press \"the left/right arrow keys\" to turn the craft." << endl
		 << "Press \"the up/down arrow keys\" to move the craft." << endl
		 << "Press \"+/-\" to increase/decrease the animation speed." << endl
		 << "Scroll \"the mouse wheel up/down\" to zoom in / out." << endl
		 << "Press \"space\" to toggle the orbits drawing." << endl
		 << "Press \"p\" to pause/unpause the animation." << endl
		 << "Press \"r\" to reset spacecraft, animation speed & zooming.";
}

// Main routine
int main(int argc, char** argv) {
	printInteraction();
	glutInit(&argc, argv);
	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 400);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Solar System.cpp");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyInput);
	glutSpecialFunc(specialKeyInput);
	setup();
	glutMainLoop();
}

