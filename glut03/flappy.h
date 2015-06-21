#pragma once
#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include "GLMetaseq.h"
#include <GL/glut.h>
#include <cmath>

class Game {
public:
	static const double unitLength;
	static double speed;
	static double screenCenter;
	static double counterCenter;
	static unsigned char viewMode;
	static int state;
	static int startTime;
	static int currentTime;
	static int prevTime;
	static int diffTime;
	static int cleared;
	static int digit;
	static int counterDispList;
	static MQO_SEQUENCE number;
	static MQO_MODEL GetReady;
	static MQO_MODEL GameOver;
	Game();
	static void frameStart();
	static void frameEnd();
	static void init();
	static void info();
	static void cleanup();
	static void reach();
	static void start();
	static void crash();
	static void reset();
	static void changeView();
	static void changeView(unsigned char mode);
	static unsigned char getViewMode();
	static int getState();
};

class Pipe {
public:
	static double radius;
	static double hitRadius;
	static double interval;
	static double opening;
	static double totalHeight;
	static double appearPos;
	static const int rowWidth;
	static const int rowNumber;
	static double bottomHeight[];
	static double position[];
	static bool reached[];
	static int rowDispList[];
	static bool disableTrans;
	Pipe();
	static void init();
	static void toggleEnableTrans();
	static void element(double tall);
	static void row(double bottomHeight);
	static void sequence();
	static void calcPosition();
	static double newHeight();
};

class Floor {
public:
	static double width;
	static double height;
	static double depth;
	static int repeat;
	static double totalWidth;
	static double initialOffset;
	static double offset;
	static int grassDispList;
	static int wallDispList;
	Floor();
	static void init();
	static void element(int color);
	static void sequence();
	static void grass();
	static void wall();
};

class Bird {
public:
	static double r;
	static double y;
	static double v;
	static double angle;
	static double iniY;
	static int timer;
	static int frame;
	static MQO_SEQUENCE flappy;
	Bird();
	static void model();
	static void calc();
	static void fall();
	static void flap();
};

#define REACH 0
#define FLAP 1
#define CRASH 2
#define FALL 3
#define FLY 4

class Sound {
public:
	static bool trigger[];
	static void player();
	static void play(int value);
};

inline double CalDetMat4x4(double m[])
{
	return m[0] * m[5] * m[10] * m[15] + m[0] * m[6] * m[11] * m[13] + m[0] * m[7] * m[9] * m[14]
		+ m[1] * m[4] * m[11] * m[14] + m[1] * m[6] * m[8] * m[15] + m[1] * m[7] * m[10] * m[12]
		+ m[2] * m[4] * m[9] * m[15] + m[2] * m[5] * m[11] * m[12] + m[2] * m[7] * m[8] * m[13]
		+ m[3] * m[4] * m[10] * m[13] + m[3] * m[5] * m[8] * m[14] + m[3] * m[6] * m[9] * m[12]
		- m[0] * m[5] * m[11] * m[14] - m[0] * m[6] * m[9] * m[15] - m[0] * m[7] * m[10] * m[13]
		- m[1] * m[4] * m[10] * m[15] - m[1] * m[6] * m[11] * m[12] - m[1] * m[7] * m[8] * m[14]
		- m[2] * m[4] * m[11] * m[13] - m[2] * m[5] * m[8] * m[15] - m[2] * m[7] * m[9] * m[12]
		- m[3] * m[4] * m[9] * m[14] - m[3] * m[5] * m[10] * m[12] - m[3] * m[6] * m[8] * m[13];
}

inline bool CalInvMat4x4(double m[], double invm[])
{
	double det = CalDetMat4x4(m);
	if (fabs(det) == 0){
		return false;
	}
	else{
		double inv_det = 1.0 / det;

		invm[0] = inv_det*(m[5] * m[10] * m[15] + m[6] * m[11] * m[13] + m[7] * m[9] * m[14] - m[5] * m[11] * m[14] - m[6] * m[9] * m[15] - m[7] * m[10] * m[13]);
		invm[1] = inv_det*(m[1] * m[11] * m[14] + m[2] * m[9] * m[15] + m[3] * m[10] * m[13] - m[1] * m[10] * m[15] - m[2] * m[11] * m[13] - m[3] * m[9] * m[14]);
		invm[2] = inv_det*(m[1] * m[6] * m[15] + m[2] * m[7] * m[13] + m[3] * m[5] * m[14] - m[1] * m[7] * m[14] - m[2] * m[5] * m[15] - m[3] * m[6] * m[13]);
		invm[3] = inv_det*(m[1] * m[7] * m[10] + m[2] * m[5] * m[11] + m[3] * m[6] * m[9] - m[1] * m[6] * m[11] - m[2] * m[7] * m[9] - m[3] * m[5] * m[10]);

		invm[4] = inv_det*(m[4] * m[11] * m[14] + m[6] * m[8] * m[15] + m[7] * m[10] * m[12] - m[4] * m[10] * m[15] - m[6] * m[11] * m[12] - m[7] * m[8] * m[14]);
		invm[5] = inv_det*(m[0] * m[10] * m[15] + m[2] * m[11] * m[12] + m[3] * m[8] * m[14] - m[0] * m[11] * m[14] - m[2] * m[8] * m[15] - m[3] * m[10] * m[12]);
		invm[6] = inv_det*(m[0] * m[7] * m[14] + m[2] * m[4] * m[15] + m[3] * m[6] * m[12] - m[0] * m[6] * m[15] - m[2] * m[7] * m[12] - m[3] * m[4] * m[14]);
		invm[7] = inv_det*(m[0] * m[6] * m[11] + m[2] * m[7] * m[8] + m[3] * m[4] * m[10] - m[0] * m[7] * m[10] - m[2] * m[4] * m[11] - m[3] * m[6] * m[8]);

		invm[8] = inv_det*(m[4] * m[9] * m[15] + m[5] * m[11] * m[12] + m[7] * m[8] * m[13] - m[4] * m[11] * m[13] - m[5] * m[8] * m[15] - m[7] * m[9] * m[12]);
		invm[9] = inv_det*(m[0] * m[11] * m[13] + m[1] * m[8] * m[15] + m[3] * m[9] * m[12] - m[0] * m[9] * m[15] - m[1] * m[11] * m[12] - m[3] * m[8] * m[13]);
		invm[10] = inv_det*(m[0] * m[5] * m[15] + m[1] * m[7] * m[12] + m[3] * m[4] * m[13] - m[0] * m[7] * m[13] - m[1] * m[4] * m[15] - m[3] * m[5] * m[12]);
		invm[11] = inv_det*(m[0] * m[7] * m[9] + m[1] * m[4] * m[11] + m[3] * m[5] * m[8] - m[0] * m[5] * m[11] - m[1] * m[7] * m[8] - m[3] * m[4] * m[9]);

		invm[12] = inv_det*(m[4] * m[10] * m[13] + m[5] * m[8] * m[14] + m[6] * m[9] * m[12] - m[4] * m[9] * m[14] - m[5] * m[10] * m[12] - m[6] * m[8] * m[13]);
		invm[13] = inv_det*(m[0] * m[9] * m[14] + m[1] * m[10] * m[12] + m[2] * m[8] * m[13] - m[0] * m[10] * m[13] - m[1] * m[8] * m[14] - m[2] * m[9] * m[12]);
		invm[14] = inv_det*(m[0] * m[6] * m[13] + m[1] * m[4] * m[14] + m[2] * m[5] * m[12] - m[0] * m[5] * m[14] - m[1] * m[6] * m[12] - m[2] * m[4] * m[13]);
		invm[15] = inv_det*(m[0] * m[5] * m[10] + m[1] * m[6] * m[8] + m[2] * m[4] * m[9] - m[0] * m[6] * m[9] - m[1] * m[4] * m[10] - m[2] * m[5] * m[8]);

		return true;
	}
}