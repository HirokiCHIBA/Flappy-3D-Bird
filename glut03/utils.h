#pragma once
#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <GL/glut.h>
#include <cmath>

#define toRadian(deg) (((deg) / 360) * 2 * M_PI)
#define toDegree(rad) (((rad) / 2 / M_PI) * 360)

double pow_1_3(double a);

class Camera {
public:
	static GLdouble matrix[];
	static GLdouble depth;
	static GLdouble movePerPixel;
	static GLdouble rotatePerPixel;
	static GLsizei width;
	static GLsizei height;
	static GLdouble angle;
	static void init(GLdouble x, GLdouble y, GLdouble z, GLdouble cx, GLdouble cy, GLdouble cz, GLdouble ux, GLdouble uy, GLdouble uz);
	static void set(int width, int height, double angle, double zNear, double zFar);
	static void attachMatrix();
	static void setDepth();
	static void calcPerPixel();
	static void changeAngle(int dx, int dy);
	static void changePosition(int dx, int dy, int dz);
	static void changePosition(double dx, double dy, double dz);
	static void renewCoordinates(GLdouble m[]);
};

class Mouse {
public:
	static int x;
	static int y;
	static int x0;
	static int y0;
	static int dx;
	static int dy;
	static int button;
	static bool down;
	Mouse();
	static void click(int button, int state, int x, int y);
	static void motion(int x, int y);
	void(*pMotion)(int x, int y) = motion;
	void(*pClick)(int button, int state, int x, int y) = click;
};
