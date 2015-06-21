#pragma once
#define _USE_MATH_DEFINES
#include <Windows.h>
#include <MMSystem.h>
#include <random>
#include <future>
#include "GLMetaseq.h"
#include "ppmload.h"
#include "utils.h"
#include "flappy.h"
#pragma comment(lib,"winmm")

//-----------------------------------------------------------------------------
// ゲーム管理

const double Game::unitLength = 0.0909;
double Game::speed;
double Game::screenCenter;
double Game::counterCenter;
unsigned char Game::viewMode;
int Game::state;
int Game::currentTime;
int Game::startTime;
int Game::prevTime;
int Game::diffTime;
int Game::cleared;
int Game::digit;
int Game::counterDispList;
MQO_SEQUENCE Game::number;
MQO_MODEL Game::GetReady;
MQO_MODEL Game::GameOver;

// コンストラクタ
Game::Game() {
	Game::speed = -Game::unitLength * 0.03023;		// interval / 1323
	Game::screenCenter = Game::unitLength * 13;
	Game::counterCenter = 0;
	Game::viewMode = '1';
	Game::state = 0;
	Game::currentTime = 0;
	Game::startTime = 0;
	Game::prevTime = 0;
	Game::diffTime = 0;
	Game::cleared = 0;
	Game::digit = 1;
}

// フレームスタート時
void Game::frameStart() {
	Game::currentTime = glutGet(GLUT_ELAPSED_TIME);
	Game::diffTime = Game::currentTime - Game::prevTime;
}

// フレーム終わり時
void Game::frameEnd() {
	Game::prevTime = Game::currentTime;
}

// 初期化
void Game::init() {
	std::thread th(Sound::player);
	th.detach();

	mqoInit();
	Bird::flappy = mqoCreateSequence("flappy_%05d.mqo", 40, 0.00025);
	Game::number = mqoCreateSequence("n%d.mqo", 10, 0.01);
	Game::GetReady = mqoCreateModel("getready.mqo", 0.01);
	Game::GameOver = mqoCreateModel("gameover.mqo", 0.01);

	Game::counterDispList = glGenLists(1);
	glNewList(Game::counterDispList, GL_COMPILE);
	mqoCallSequence(Game::number, 0);
	glEndList();

	Pipe::init();
	Floor::init();
}

// 情報表示
void Game::info() {
	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT1);


	glPushMatrix();
	glLoadIdentity();
	if (Game::state == 0) {
		glPushMatrix();
		glTranslated(0.0, 0.2, -1.0);
		glScaled(0.1, 0.1, 0.1);
		mqoCallModel(Game::GetReady);
		glPopMatrix();
	}
	else if (Game::state == 2) {
		glPushMatrix();
		glTranslated(0.0, 0.2, -1.0);
		glScaled(0.1, 0.1, 0.1);
		mqoCallModel(Game::GameOver);
		glPopMatrix();
	}
	glPushMatrix();
	glTranslated(Game::counterCenter * 0.1, 0.32, -1.0);
	glScaled(0.1, 0.1, 0.1);
	glCallList(Game::counterDispList);
	glPopMatrix();
	glPopMatrix();

	glDisable(GL_LIGHT1);
	glEnable(GL_LIGHT0);
}

// mqoをクリーンアップ
void Game::cleanup() {
	mqoDeleteSequence(Bird::flappy);
	mqoDeleteSequence(Game::number);
	mqoDeleteModel(Game::GetReady);
	mqoDeleteModel(Game::GameOver);
	mqoCleanup();
}

// 得点
void Game::reach() {
	Game::cleared++;
	if (Game::cleared == std::pow(10, Game::digit)) {
		Game::digit++;
	}

	int tmp = Game::cleared;
	double width = 0.05;
	double firstWidth = 0;
	double prevPos = 0;
	
	glDeleteLists(Game::counterDispList, 1);
	Game::counterDispList = glGenLists(1);
	glNewList(Game::counterDispList, GL_COMPILE);
	for (int i = 0; i < Game::digit; i++) {
		int n = tmp % (int)std::pow(10, i + 1);

		tmp -= n;
		n /= (int)std::pow(10, i);

		double currPos = 0;
		double currWidth = (n == 1) ? 0.125 : 0.225;			// 数字(白色部)の横幅の半分

		if (i > 0) {
			currPos = prevPos + currWidth;				// 前の数字の左端(黒色部含む) + 自分の横幅の半分
		}
		else {
			firstWidth = currWidth;
		}
		
		glPushMatrix();
		glTranslated(-currPos, 0.0, 0.0);
		mqoCallSequence(Game::number, n);
		glPopMatrix();

		width += currWidth * 2 + 0.05;					// 全体の横幅に加算
		prevPos += currWidth + 0.05;						// 左端の位置を計算
	}
	glEndList();
	Game::counterCenter = width / 2 - firstWidth - 0.05;
}

// ゲームスタート
void Game::start() {
	Game::state = 1;
	Game::startTime = glutGet(GLUT_ELAPSED_TIME);
	Game::changeView();
}

// クラッシュ
void Game::crash() {
	Game::state = 2;
}

// リセット
void Game::reset() {
	Game::state = 0;
	Game::cleared = 0;
	Game::digit = 1;
	Game::counterCenter = 0;
	Pipe::init();
	Bird::angle = 0;
	Sound::play(FLY);

	glDeleteLists(Game::counterDispList, 1);
	Game::counterDispList = glGenLists(1);
	glNewList(Game::counterDispList, GL_COMPILE);
	mqoCallSequence(Game::number, 0);
	glEndList();

	Bird::y = Bird::iniY;
	Game::changeView();
}

// 視点変更
void Game::changeView() {
	switch (Game::viewMode) {
	case '1':
		Camera::init(Game::screenCenter, Game::unitLength * 35, 12.0, Game::screenCenter, Game::unitLength * 35, 0.0, 0.0, 1.0, 0.0);
		Camera::attachMatrix();
		break;
	case '2':
		Camera::init(-3.0, Bird::y, 3.0, Bird::r, Bird::y, 0.0, 0.0, 1.0, 0.0);
		Camera::attachMatrix();
		break;
	case '3':
		Camera::attachMatrix();
		break;
	}
}

// 視点更新
void Game::changeView(unsigned char mode) {
	Game::viewMode = mode;
	Game::changeView();
	Sound::play(FLY);
}

// 視点モード取得
unsigned char Game::getViewMode() {
	return Game::viewMode;
}

// 状態取得
int Game::getState() {
	return Game::state;
}

//-----------------------------------------------------------------------------
// 土管

double Pipe::radius;
double Pipe::hitRadius;
double Pipe::interval;
double Pipe::opening;
double Pipe::totalHeight;
double Pipe::appearPos;
const int Pipe::rowWidth = 5;
const int Pipe::rowNumber = 7;
double Pipe::bottomHeight[Pipe::rowNumber];
double Pipe::position[Pipe::rowNumber];
bool Pipe::reached[Pipe::rowNumber];
int Pipe::rowDispList[Pipe::rowNumber];
bool Pipe::disableTrans;

// コンストラクタ
Pipe::Pipe() {
	Pipe::radius = Game::unitLength * 6;
	Pipe::hitRadius = Game::unitLength * 5.5;
	Pipe::interval = Game::unitLength * 40;
	Pipe::opening = Game::unitLength * 24;
	Pipe::totalHeight = Game::unitLength * 108;				// 90
	Pipe::appearPos = Game::unitLength * 156;				// 59
	Pipe::disableTrans = false;
}

// 初期化
void Pipe::init() {
	//static double initPos = -Game::speed * 4168;
	for (int i = 0; i < Pipe::rowNumber; i++) {
		Pipe::bottomHeight[i] = Pipe::newHeight();
		Pipe::position[i] = Pipe::appearPos + Pipe::interval * i;
		Pipe::reached[i] = false;

		glDeleteLists(Pipe::rowDispList[i], 1);
		Pipe::rowDispList[i] = glGenLists(1);
		glNewList(Pipe::rowDispList[i], GL_COMPILE);
		Pipe::row(Pipe::bottomHeight[i]);
		glEndList();
	}
}

// 透過の有無の切替
void Pipe::toggleEnableTrans() {
	Pipe::disableTrans = !Pipe::disableTrans;
}

// 土管(1本)の描画
void Pipe::element(double tall) {
	static int n = 72;
	static double theta = 2.0 * M_PI / n;
	static double innerRad = Game::unitLength * 4.5;
	static double outerRad = Pipe::hitRadius;
	static double edgeRad = Pipe::radius;
	static double edgeTall = Game::unitLength * 4;
	static double cos = std::cos(theta);
	static double sin = std::sin(theta);
	static double coshalf = std::cos(theta / 2);
	static double sinhalf = std::sin(theta / 2);

	GLdouble vertex[][3] = {
		{ innerRad, 0.0, 0.0 },
		{ innerRad * cos, 0.0, innerRad * sin },
		{ innerRad * cos, tall, innerRad * sin },
		{ innerRad, tall, 0.0 },
		{ edgeRad, tall, 0.0 },
		{ edgeRad * cos, tall, edgeRad * sin },
		{ edgeRad * cos, tall - edgeTall, edgeRad * sin },
		{ edgeRad, tall - edgeTall, 0.0 },
		{ outerRad, tall - edgeTall, 0.0 },
		{ outerRad * cos, tall - edgeTall, outerRad * sin },
		{ outerRad * cos, 0.0, outerRad * sin },
		{ outerRad, 0.0, 0.0 },
	};

	static int face[][4] = {
		{ 0, 1, 2, 3 },
		{ 5, 4, 3, 2 },
		{ 4, 5, 6, 7 },
		{ 9, 8, 7, 6 },
		{ 8, 9, 10, 11 }
	};

	static GLdouble normal[][3] = {
		{ -coshalf, 0.0, -sinhalf },
		{ 0.0, 1.0, 0.0 },
		{ coshalf, 0.0, sinhalf },
		{ 0.0, -1.0, 0.0 },
		{ coshalf, 0.0, sinhalf }
	};

	for (int i = 0; i < n; i++) {
		glPushMatrix();
		glRotated(-toDegree(theta * i), 0.0, 1.0, 0.0);

		glBegin(GL_QUADS);
		for (int j = 0; j < 5; j++) {
			glNormal3dv(normal[j]);
			for (int k = 0; k < 4; k++)
			{
				glVertex3dv(vertex[face[j][k]]);
			}
		}
		glEnd();
		glPopMatrix();
	}
}

// 土管(一列)の描画
void Pipe::row(double bottomHeight) {
	static double offset = Pipe::radius * (Pipe::rowWidth - 1);

	for (int i = 0; i < rowWidth; i++) {
		glPushMatrix();
		glTranslated(0.0, 0.0, Pipe::radius * 2 * i - offset);
		Pipe::element(bottomHeight);
		glPopMatrix();
	}
	for (int i = 0; i < rowWidth; i++) {
		glPushMatrix();
		glRotated(180.0, 0.0, 0.0, 1.0);
		glTranslated(0.0, - Pipe::totalHeight, Pipe::radius * 2 * i - offset);
		Pipe::element(Pipe::totalHeight - Pipe::opening - bottomHeight);
		glPopMatrix();
	}
}

// 土管(全体)の描画
void Pipe::sequence() {
	static GLfloat specular[][4] = {
		{ 0.5f, 0.5f, 0.4f, 1.0f },
		{ 0.5f, 0.5f, 0.4f, 0.3f }
	};
	static GLfloat diffuse[][4] = {
		{ 0.4627f, 0.6f, 0.1843f, 1.0f },
		{ 0.4627f, 0.6f, 0.1843f, 0.3f }
	};
	static GLfloat shininess[1] = { 10.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	Pipe::calcPosition();
	for (int i = Pipe::rowNumber - 1; i >= 0 ; i--) {
		bool notTrans = Pipe::disableTrans || !Pipe::reached[i];
		if (notTrans) {
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffuse[0]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular[0]);
		}
		else {
			glDepthMask(FALSE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffuse[1]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular[1]);
		}
		glPushMatrix();
		glTranslated(Pipe::position[i], 0.0, 0.0);
		glCallList(Pipe::rowDispList[i]);
		glPopMatrix();
		if (!notTrans) {
			glDepthMask(TRUE);
		}
	}
}

// 土管の位置を計算・当たり判定
void Pipe::calcPosition() {
	if (Game::state == 1) {
		double dx = Game::speed * Game::diffTime;
		for (int i = 0; i < Pipe::rowNumber; i++) {
			double cx = Pipe::position[i] + dx;
			if (cx < Pipe::appearPos - Pipe::interval * Pipe::rowNumber) {
				Pipe::bottomHeight[i] = Pipe::newHeight();
				Pipe::position[i] = Pipe::appearPos;
				Pipe::reached[i] = false;

				glDeleteLists(Pipe::rowDispList[i], 1);
				Pipe::rowDispList[i] = glGenLists(1);
				glNewList(Pipe::rowDispList[i], GL_COMPILE);
				Pipe::row(Pipe::bottomHeight[i]);
				glEndList();
			}
			else {
				Pipe::position[i] = cx;
				if (cx < 0.0) {
					if (!Pipe::reached[i]) {
						Pipe::reached[i] = true;
						Game::reach();
						Sound::play(REACH);
					}
				}
				if (-(Pipe::hitRadius + Bird::r) < cx && cx < Pipe::hitRadius + Bird::r) {
					if (Bird::y <= Pipe::bottomHeight[i] + Bird::r || Pipe::bottomHeight[i] + Pipe::opening - Bird::r <= Bird::y) {
						Game::crash();
						Sound::play(FALL);
					}
				}
			}
		}
	}
}

// 土管の隙間の高さをランダムに生成
double Pipe::newHeight() {
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static std::uniform_int_distribution<int> randOpening(15, (int)((Pipe::totalHeight - Pipe::opening) / Game::unitLength) - 33);

	return Game::unitLength * randOpening(mt);
}

//-----------------------------------------------------------------------------
// 地面等

double Floor::width;
double Floor::height;
double Floor::depth;
int Floor::repeat;
double Floor::totalWidth;
double Floor::initialOffset;
double Floor::offset;
int Floor::grassDispList;
int Floor::wallDispList;

// コンストラクタ
Floor::Floor() {
	Floor::width = Game::unitLength * 2.4;
	Floor::height = Game::unitLength * 3;
	Floor::depth = Pipe::radius * Pipe::rowWidth;
	Floor::repeat = 60;
	Floor::totalWidth = Floor::width * Floor::repeat;
	Floor::initialOffset = -Floor::totalWidth + Game::screenCenter;
	Floor::offset = Floor::initialOffset;
}

// 初期化
void Floor::init() {
	Floor::grass();
	Floor::wall();
}

// 草(1本文)の描画
void Floor::element(int color) {
	static GLfloat specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	static GLfloat diffuse[][4] = {
		{ 0.451f, 0.749f, 0.1804f, 1.0f },
		{ 0.6118f, 0.902f, 0.349f, 1.0f }
	};
	static GLfloat shininess[1] = { 10.0f };
	
	static GLdouble vertex[][3] = {
		{ Floor::width, 0.0, -Floor::depth },
		{ 0.0, 0.0, -Floor::depth },
		{ 0.0, 0.0, Floor::depth },
		{ Floor::width, 0.0, Floor::depth },
		{ 0.0, -Floor::height, Floor::depth },
		{ -Floor::width, -Floor::height, Floor::depth }
	};
	static int face[][4] = {
		{ 0, 1, 2, 3 },
		{ 5, 4, 3, 2 }
	};
	static GLdouble normal[][3] = {
		{ 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 1.0 }
	};

	glBegin(GL_QUADS);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffuse[color]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	for (int i = 0; i < 2; i++) {
		glNormal3dv(normal[i]);
		for (int j = 0; j < 4; j++)
		{
			glVertex3dv(vertex[face[i][j]]);
		}
	}
	glEnd();
}

// 地面・壁全体の描画
void Floor::sequence() {
	static double range = Floor::width * 2;

	if (Game::state < 2) {
		Floor::offset += Game::speed * Game::diffTime;
		if (Floor::initialOffset - Floor::offset >= range)
			Floor::offset += range;
	}
	glPushMatrix();
	glTranslated(Floor::offset, 0.0, 0.0);
	glCallList(Floor::grassDispList);
	glPopMatrix();

	glCallList(Floor::wallDispList);
}

//  草(全体)の描画
void Floor::grass() {
	Floor::grassDispList = glGenLists(1);
	glNewList(Floor::grassDispList, GL_COMPILE);
	for (int i = 0; i < 2 * Floor::repeat; i++) {
		double x = Floor::width * i;
		glPushMatrix();
		glTranslated(x, 0.0, 0.0);
		Floor::element(i % 2);
		glPopMatrix();
	}
	glEndList();
}

// 壁・土の描画
void Floor::wall() {
	static struct ppmimg *image = NULL;
	static double size = Game::unitLength * 27.5;
	static int repeat = std::ceil(2 * Floor::totalWidth / size);

	static bool istexloaded = false;

	if (!istexloaded){
		if (image == NULL)image = (struct ppmimg*)malloc(sizeof(ppmimg));
		image->dat = NULL;
		loadppmimage("wall.ppm", image);
		istexloaded = true;

		/* テクスチャ画像はバイト単位に詰め込まれている */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		/* テクスチャの割り当て */
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image->iwidth, image->iheight, GL_RGB, GL_UNSIGNED_BYTE, image->dat);
	}

	// 0:背景画像用 1:土用 の材質設定
	static GLfloat specular[][4] = {
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	static GLfloat diffuse[][4] = {
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.8706f, 0.8471f, 0.5843f, 1.0f }
	};

	// 背景・土共通の材質設定
	static GLfloat shininess[1] = { 0.0f };
	static GLdouble normal[3] = { 0.0, 0.0, 1.0 };

	// 土の頂点
	static GLdouble vertex[][3] = {
		{ Floor::totalWidth + Game::screenCenter, -Floor::height, Floor::depth },
		{ -Floor::totalWidth + Game::screenCenter, -Floor::height, Floor::depth },
		{ -Floor::totalWidth + Game::screenCenter, -10.0, Floor::depth },
		{ Floor::totalWidth + Game::screenCenter, -10.0, Floor::depth }
	};
	
	Floor::wallDispList = glGenLists(1);
	glNewList(Floor::wallDispList, GL_COMPILE);

	for (int i = 0; i < repeat; i++) {
		glPushMatrix();
		glTranslated(-Floor::totalWidth + size * i + Game::screenCenter, 0.0, 0.0);
		glEnable(GL_TEXTURE_2D);
		glPushMatrix();
		glBegin(GL_POLYGON);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular[0]);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffuse[0]);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
		glNormal3dv(normal);
		glTexCoord2f(1.0, 0.0); glVertex3d(size, size, -Floor::depth);
		glTexCoord2f(0.0, 0.0); glVertex3d(0.0, size, -Floor::depth);
		glTexCoord2f(0.0, 1.0); glVertex3d(0.0, 0.0, -Floor::depth);
		glTexCoord2f(1.0, 1.0); glVertex3d(size, 0.0, -Floor::depth);
		glEnd();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}

	glPushMatrix();
	glBegin(GL_QUADS);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular[1]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffuse[1]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	glNormal3dv(normal);
	for (int i = 0; i < 4; i++)
	{
		glVertex3dv(vertex[i]);
	}
	glEnd();
	glPopMatrix();

	glEndList();
}

//-----------------------------------------------------------------------------
// 鳥

double Bird::r;
double Bird::y;
double Bird::v;
double Bird::angle;
double Bird::iniY;
int Bird::timer;
int Bird::frame;
MQO_SEQUENCE Bird::flappy;

// コンストラクタ
Bird::Bird() {
	Bird::r = Game::unitLength * 3;
	Bird::y = 0;
	Bird::v = 0;
	Bird::angle = 0;
	Bird::iniY = Game::unitLength * 35;
	Bird::timer = 0;
	Bird::frame = 0;
}

// 鳥の描画
void Bird::model() {
	calc();

	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glPushMatrix();
	glTranslated(0.0, Bird::y, 0.0);
	// glutWireCube(Bird::r * 2);
	glPushMatrix();
	glRotated(Bird::angle, 0.0, 0.0, 1.0);
	mqoCallSequence(Bird::flappy, Bird::frame);
	glPopMatrix();
	glPopMatrix();

	glDisable(GL_LIGHT1);
	glEnable(GL_LIGHT0);
}

// 位置・角度・アニメフレームの計算
void Bird::calc() {
	Bird::timer += Game::diffTime;
	switch (Game::getState()) {
	case 0:
		{
			static double omega = 2 * M_PI / 750;
			static double a = Game::unitLength * 3.5;
			Bird::y = a * std::sin(omega * Game::currentTime) + Bird::iniY;
			if (Bird::timer > 33) {
				Bird::timer = 0;
				Bird::frame = (Bird::frame + 5) % 40;
			}
		}
		break;
	case 1:
		{
			if (Bird::y > Bird::r) {
				Bird::fall();
			}
			else {
				Game::crash();
				Sound::play(CRASH);
			}
		}
		break;
	case 2:
		if (Bird::y > Bird::r) {
			Bird::fall();
		}
		break;
	}
}

// 落下(+角度・アニメフレーム)の計算
void Bird::fall() {
	double dt = Game::diffTime / 1000.0;
	Bird::v -= 25 * dt;
	double dy = v * dt;
	Bird::y += dy;
	if (Game::getViewMode() == '2') {
		Camera::changePosition(0.0, -dy, 0.0);
	}
	if (v > -4) {
		if (Bird::timer > 16) {
			Bird::timer = 0;
			Bird::frame = (Bird::frame + 8) % 40;
		}
		if (Bird::angle < 20) {
			Bird::angle += toDegree(std::atan2(Bird::v / 60, Bird::r));
			if (Bird::angle > 20)
				Bird::angle = 20;
		}
		else {
			Bird::angle = 20;
		}
	}
	else {
		Bird::frame = 0;
		if (Bird::angle > -90) {
			Bird::angle += toDegree(std::atan2(Bird::v / 140, Bird::r));
			if (Bird::angle < -90)
				Bird::angle = -90;
		}
	}
}

// 羽撃き
void Bird::flap() {
	Bird::v = 8;
	Bird::frame = 10;
	Bird::angle += 20;
	Sound::play(FLAP);
}

//-----------------------------------------------------------------------------
// 音声

bool Sound::trigger[5] = { false };

// 音声を再生(別スレッドでトリガを監視)
void Sound::player() {
	static MCI_OPEN_PARMS mop1, mop2, mop3, mop4, mop5;
	static MCI_STATUS_PARMS msp;

	mop1.lpstrDeviceType = TEXT("WaveAudio");
	mop1.lpstrElementName = TEXT("chime.wav");
	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mop1);

	mop2.lpstrDeviceType = TEXT("WaveAudio");
	mop2.lpstrElementName = TEXT("flap.wav");
	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mop2);

	mop3.lpstrDeviceType = TEXT("WaveAudio");
	mop3.lpstrElementName = TEXT("crash.wav");
	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mop3);

	mop4.lpstrDeviceType = TEXT("WaveAudio");
	mop4.lpstrElementName = TEXT("fall.wav");
	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mop4);

	mop5.lpstrDeviceType = TEXT("WaveAudio");
	mop5.lpstrElementName = TEXT("fly.wav");
	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mop5);

	msp.dwItem = MCI_STATUS_MODE;

	while (1) {
		if (Sound::trigger[REACH]) {
			mciSendCommand(mop1.wDeviceID, MCI_SEEK, MCI_SEEK_TO_START, 0);
			mciSendCommand(mop1.wDeviceID, MCI_PLAY, 0, 0);
			Sound::trigger[REACH] = false;
		}

		if (Sound::trigger[FLAP]) {
			mciSendCommand(mop2.wDeviceID, MCI_SEEK, MCI_SEEK_TO_START, 0);
			mciSendCommand(mop2.wDeviceID, MCI_PLAY, 0, 0);
			Sound::trigger[FLAP] = false;
		}

		if (Sound::trigger[CRASH]) {
			mciSendCommand(mop3.wDeviceID, MCI_SEEK, MCI_SEEK_TO_START, 0);
			mciSendCommand(mop3.wDeviceID, MCI_PLAY, 0, 0);
			Sound::trigger[CRASH] = false;
		}

		if (Sound::trigger[FALL]) {
			mciSendCommand(mop4.wDeviceID, MCI_SEEK, MCI_SEEK_TO_START, 0);
			mciSendCommand(mop4.wDeviceID, MCI_PLAY, 0, 0);
			Sound::trigger[FALL] = false;
		}

		if (Sound::trigger[FLY]) {
			mciSendCommand(mop5.wDeviceID, MCI_SEEK, MCI_SEEK_TO_START, 0);
			mciSendCommand(mop5.wDeviceID, MCI_PLAY, 0, 0);
			Sound::trigger[FLY] = false;
		}
	}
}

// 再生トリガを立てる
void Sound::play(int value) {
	Sound::trigger[value] = true;
}