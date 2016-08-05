/*visBool3d
Copyright (C) 2013 Michel Dubois

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <png.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define WINDOW_TITLE_PREFIX "Visualize Boolean"
#define couleur(param) printf("\033[%sm",param)


typedef struct _bar {
	long x, y, h;
	GLfloat r, g, b;
} bar;


static short winSizeW = 920,
	winSizeH = 690,
	frame = 0,
	currentTime = 0,
	timebase = 0,
	fullScreen = 0,
	rotate = 0,
	dt = 20; // in milliseconds


static int textList = 0,
	objectList = 0,
	cpt = 0;


static float fps = 0.0,
	rotx = -80.0,
	roty = 0.0,
	rotz = 20.0,
	xx = 0.0,
	yy = 5.0,
	zoom = 100.0,
	prevx = 0.0,
	prevy = 0.0,
	factor = 0.0;


static bar *barsList = NULL;
static unsigned long sampleSize = 0;
static char *func = NULL;
static char *barOrCube = NULL;


void usage(void) {
	couleur("31");
	printf("Michel Dubois -- visBool3d -- (c) 2013\n\n");
	couleur("0");
	printf("Syntaxe: visBool3d <samplesize> <type> <function>\n");
	printf("\t<samplesize> -> size of the sample space\n");
	printf("\t<type> -> type of visualization\n");
	printf("\t\tavalaible type are: bar, cube\n");
	printf("\t<function> -> name of the boolean function\n");
	printf("\t\tavalaible boolean functions are: xor, and, or, xnor, nand, nor, alea\n");
}


void takeScreenshot(char *filename) {
	FILE *fp = fopen(filename, "wb");
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);
	unsigned char *buffer = calloc((width * height * 3), sizeof(unsigned char));
	int i;

	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)buffer);
	png_init_io(png, fp);
	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);
	for (i=0; i<height; i++) {
		png_write_row(png, &(buffer[3*width*((height-1) - i)]));
	}
	png_write_end(png, NULL);
	png_destroy_write_struct(&png, &info);
	free(buffer);
	fclose(fp);
	printf("INFO: Save screenshot on %s (%d x %d)\n", filename, width, height);
}


void drawString(float x, float y, float z, char *text) {
	unsigned i = 0;
	glPushMatrix();
	glLineWidth(1.0);
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(x, y, z);
	glScalef(0.01, 0.01, 0.01);
	for(i=0; i < strlen(text); i++) {
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, (int)text[i]);
	}
	glPopMatrix();
}


void drawText(void) {
	char text1[50], text2[70];
	sprintf(text1, "dt: %1.3f, FPS: %4.2f", (dt/1000.0), fps);
	sprintf(text2, "Size length: %ld, Function: %s", sampleSize, func);
	textList = glGenLists(1);
	glNewList(textList, GL_COMPILE);
	glLineWidth(1.0);
	glColor3f(1.0, 1.0, 1.0);
	drawString(-40.0, -38.0, -100.0, text1);
	drawString(-40.0, -36.0, -100.0, text2);
	glEndList();
}


void drawAxes(void) {
	float rayon = 0.1;
	float length = 100/4.0;

	// cube
	glPushMatrix();
	glLineWidth(1.0);
	glColor3f(0.8, 0.8, 0.8);
	glTranslatef(0.0, 0.0, 0.0);
	glutWireCube(100.0/2.0);
	glPopMatrix();

	// origin
	glPushMatrix();
	glColor3f(1.0, 1.0, 1.0);
	glutSolidSphere(rayon*4, 16, 16);
	glPopMatrix();

	// x axis
	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0);
	glTranslatef(length/2.0, 0.0, 0.0);
	glScalef(length*5.0, 1.0, 1.0);
	glutSolidCube(rayon*2.0);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(length, 0.0, 0.0);
	glRotated(90, 0, 1, 0);
	glutSolidCone(rayon*2, rayon*4, 8, 8);
	glPopMatrix();
	drawString(length+2.0, 0.0, 0.0, "X");

	// y axis
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	glTranslatef(0.0, length/2.0, 0.0);
	glScalef(1.0, length*5.0, 1.0);
	glutSolidCube(rayon*2.0);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0, length, 0.0);
	glRotated(90, -1, 0, 0);
	glutSolidCone(rayon*2, rayon*4, 8, 8);
	glPopMatrix();
	drawString(0.0, length+2.0, 0.0, "Y");

	// z axis
	glPushMatrix();
	glColor3f(0.0, 0.0, 1.0);
	glTranslatef(0.0, 0.0, length/2.0);
	glScalef(1.0, 1.0, length*5.0);
	glutSolidCube(rayon*2.0);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0, 0.0, length);
	glRotated(90, 0, 0, 1);
	glutSolidCone(rayon*2, rayon*4, 8, 8);
	glPopMatrix();
	drawString(0.0, 0.0, length+2.0, "Z");
}


void drawBar(long x, long y, long h) {
	int i = 0;
	float fx=0.0, fy=0.0, fh=0.0, decal=sampleSize/2.0;
	for (i=0; i<h; i++) {
		fx = (x-decal)*factor;
		fy = (y-decal)*factor;
		fh = (i-decal)*factor;
		glPushMatrix();
		glTranslatef(fx, fy, fh);
		glutSolidCube(factor);
		glPopMatrix();
	}
}


void drawCube(long x, long y, long h) {
	float fx=0.0, fy=0.0, fh=0.0, decal=sampleSize/2.0;
	fx = (x-decal)*factor;
	fy = (y-decal)*factor;
	fh = (h-decal)*factor;
	glPushMatrix();
	glTranslatef(fx, fy, fh);
	glutSolidCube(factor);
	glPopMatrix();
}


void drawObject(void) {
	unsigned long i = 0;
	objectList = glGenLists(1);
	glNewList(objectList, GL_COMPILE_AND_EXECUTE);
	glLineWidth(1.0);
	for (i=0; i<pow(sampleSize, 2); i++) {
		glPushMatrix();
		glColor3f(barsList[i].r, barsList[i].g, barsList[i].b);
		if (strcmp(barOrCube, "bar") == 0) {
			drawBar(barsList[i].x, barsList[i].y, barsList[i].h);
		} else if (strcmp(barOrCube, "cube") == 0) {
			drawCube(barsList[i].x, barsList[i].y, barsList[i].h);
		} else {
			usage();
			exit(EXIT_FAILURE);
		}
		glPopMatrix();
	}
	glEndList();
}


void onReshape(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, width/height, 1.0, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void onSpecial(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_UP:
			rotx += 5.0;
			printf("INFO: x = %f\n", rotx);
			break;
		case GLUT_KEY_DOWN:
			rotx -= 5.0;
			printf("INFO: x = %f\n", rotx);
			break;
		case GLUT_KEY_LEFT:
			rotz += 5.0;
			printf("INFO: z = %f\n", rotz);
			break;
		case GLUT_KEY_RIGHT:
			rotz -= 5.0;
			printf("INFO: z = %f\n", rotz);
			break;
		default:
			printf("x %d, y %d\n", x, y);
			break;
	}
	glutPostRedisplay();
}


void onMotion(int x, int y) {
	if (prevx) {
		xx += ((x - prevx)/10.0);
		printf("INFO: x = %f\n", xx);
	}
	if (prevy) {
		yy -= ((y - prevy)/10.0);
		printf("INFO: y = %f\n", yy);
	}
	prevx = x;
	prevy = y;
	glutPostRedisplay();
}


void onIdle(void) {
	frame += 1;
	currentTime = glutGet(GLUT_ELAPSED_TIME);
	if (currentTime - timebase >= 1000.0){
		fps = frame*1000.0 / (currentTime-timebase);
		timebase = currentTime;
		frame = 0;
	}
	glutPostRedisplay();
}


void onMouse(int button, int state, int x, int y) {
	switch (button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN) {
				printf("INFO: left button, x %d, y %d\n", x, y);
			}
			break;
		case GLUT_RIGHT_BUTTON:
			if (state == GLUT_DOWN) {
				printf("INFO: right button, x %d, y %d\n", x, y);
			}
			break;
	}
}


void onKeyboard(unsigned char key, int x, int y) {
	char *name = malloc(20 * sizeof(char));
	switch (key) {
		case 27: // Escape
			printf("INFO: exit\n");
			printf("x %d, y %d\n", x, y);
			exit(0);
			break;
		case 'x':
			xx += 1.0;
			printf("INFO: x = %f\n", xx);
			break;
		case 'X':
			xx -= 1.0;
			printf("INFO: x = %f\n", xx);
			break;
		case 'y':
			yy += 1.0;
			printf("INFO: y = %f\n", yy);
			break;
		case 'Y':
			yy -= 1.0;
			printf("INFO: y = %f\n", yy);
			break;
		case 'f':
			fullScreen = !fullScreen;
			if (fullScreen) {
				glutFullScreen();
			} else {
				glutReshapeWindow(winSizeW, winSizeH);
				glutPositionWindow(100,100);
				printf("INFO: fullscreen %d\n", fullScreen);
			}
			break;
		case 'r':
			rotate = !rotate;
			printf("INFO: rotate %d\n", rotate);
			break;
		case 'z':
			zoom -= 5.0;
			if (zoom < 5.0) {
				zoom = 5.0;
			}
			printf("INFO: zoom = %f\n", zoom);
			break;
		case 'Z':
			zoom += 5.0;
			printf("INFO: zoom = %f\n", zoom);
			break;
		case 'p':
			printf("INFO: take a screenshot\n");
			sprintf(name, "capture_%.3d.png", cpt);
			takeScreenshot(name);
			cpt += 1;
			break;
		default:
			break;
	}
	free(name);
	glutPostRedisplay();
}


void onTimer(int event) {
	switch (event) {
		case 0:
			break;
		default:
			break;
	}
	prevx = 0.0;
	prevy = 0.0;
	if (rotate) {
		rotz -= 0.2;
	} else {
		rotz += 0.0;
	}
	if (rotz > 360) rotz = 360;
	glutPostRedisplay();
	glutTimerFunc(dt, onTimer, 0);
}


void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	drawText();
	glCallList(textList);

	glPushMatrix();
	glTranslatef(xx, yy, -zoom);
	glRotatef(rotx, 1.0, 0.0, 0.0);
	glRotatef(roty, 0.0, 1.0, 0.0);
	glRotatef(rotz, 0.0, 0.0, 1.0);
	drawAxes();
	glCallList(objectList);
	glPopMatrix();

	glutSwapBuffers();
	glutPostRedisplay();

	glDeleteLists(textList, 1);
}


void init(void) {
	GLfloat position[] = {0.0, 0.0, 0.0, 1.0};
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	GLfloat modelAmbient[] = {0.5, 0.5, 0.5, 1.0};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, modelAmbient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	GLfloat no_mat[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat mat_diffuse[] = {0.1, 0.5, 0.8, 1.0};
	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat shininess[] = {128.0};
	glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

	glEnable(GL_NORMALIZE);
	glEnable(GL_AUTO_NORMAL);
	glDepthFunc(GL_LESS);
	//glClearColor(0.1, 0.1, 0.1, 1.0); // définition de la couler de fond
	glClearColor(0.95, 0.95, 0.95, 1.0); // définition de la couler de fond
	drawObject();
}


void glmain(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitWindowSize(winSizeW, winSizeH);
	glutInitWindowPosition(120, 10);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow(WINDOW_TITLE_PREFIX);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(onReshape);
	glutSpecialFunc(onSpecial);
	glutMotionFunc(onMotion);
	glutIdleFunc(onIdle);
	glutMouseFunc(onMouse);
	glutKeyboardFunc(onKeyboard);
	glutTimerFunc(dt, onTimer, 0);
	fprintf(stdout, "INFO: OpenGL Version: %s\n", glGetString(GL_VERSION));
	glutMainLoop();
	glDeleteLists(objectList, 1);
}


void booleanFunction(void) {
	unsigned long i=0, x=0, y=0;
	long valMax=0, valMin=0;
	const int max=sampleSize, min=0;
	bar temp;
	for (x=0; x<sampleSize; x++) {
		for (y=0; y<sampleSize; y++) {
			temp.x = x;
			temp.y = y;
			if ((temp.x % 2) ^ (temp.y % 2)) {
				temp.r = 0.733;
				temp.g = 0.640;
				temp.b = 0.430;
			} else {
				temp.r = 0.170;
				temp.g = 0.290;
				temp.b = 0.380;
			}
			if (strcmp(func, "xor") == 0) {
				temp.h = x ^ y;
			} else if (strcmp(func, "and") == 0) {
				temp.h = x & y;
			} else if (strcmp(func, "or") == 0) {
				temp.h = x | y;
			} else if (strcmp(func, "alea") == 0) {
				temp.h = (rand() % (max - min + 1)) + min; // generate a random number between min and max
			} else if (strcmp(func, "nand") == 0) {
				temp.h = ~(x & y);
			} else if (strcmp(func, "nor") == 0) {
				temp.h = ~(x | y);
			} else if (strcmp(func, "xnor") == 0) {
				temp.h = ~(x ^ y);
			} else {
				usage();
				exit(EXIT_FAILURE);
			}
			if (temp.h > valMax) { valMax = temp.h; }
			if (temp.h < valMin) { valMin = temp.h; }
			barsList[i] = temp;
			i++;
		}
	}
	printf("Max: %ld, Min: %ld\n", valMax, valMin);
}


void playSample(int argc, char *argv[]) {
	booleanFunction();
	glmain(argc, argv);
}


int main(int argc, char *argv[]) {
	switch (argc) {
		case 4:
			srand(time(NULL));
			factor = 0.5;
			sampleSize = atoi(argv[1]);
			barOrCube = argv[2];
			func = argv[3];
			barsList = malloc(pow(sampleSize, 2) * sizeof(bar));
			playSample(argc, argv);
			free(barsList);
			exit(EXIT_SUCCESS);
			break;
		default:
			usage();
			exit(EXIT_FAILURE);
			break;
		}
}
