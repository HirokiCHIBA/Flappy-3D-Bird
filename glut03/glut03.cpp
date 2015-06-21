#define _USE_MATH_DEFINES
#include "utils.h"
#include "flappy.h"

Game game;
Pipe pipe;
Floor ffloor;
Bird bird;
Mouse mouse;
Camera camera;

// 描画初期設定
void init(void){
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);

	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

// glutReshapeFunc
void reshape(int width, int height)
{
	Camera::set(width, height, 45.0, 0.1, 100.0);
	Game::changeView();

	static GLfloat lightPosition[4] = { 0.0f, Game::unitLength * 35, 12.0f, 1.0f };
	static GLfloat lightSpecular[3] = { 1.0f, 1.0f, 1.0f };
	static GLfloat lightDiffuse[3] = { 0.7f, 0.7f, 0.7f };
	static GLfloat lightAmbient[3] = { 0.2f, 0.2f, 0.2f };

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
	glEnable(GL_LIGHT0);
}

// glutDisplayFunc
void display()
{
	glClearColor(0.3059f, 0.7529f, 0.7922f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static GLfloat lightPosition[4] = { -10.0f, 10.0f, 12.0f, 1.0f };
	static GLfloat lightSpecular[3] = { 1.0f, 1.0f, 1.0f };
	static GLfloat lightDiffuse[3] = { 0.7f, 0.7f, 0.7f };
	static GLfloat lightAmbient[3] = { 0.2f, 0.2f, 0.2f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);

	Game::frameStart();
	Floor::sequence();
	Bird::model();
	Pipe::sequence();
	Game::info();
	Game::frameEnd();

	glutSwapBuffers();
}

void motion(int x, int y) {
	mouse.motion(x, y);

	if (Game::getViewMode() == '3' && mouse.down) {
		switch (mouse.button) {
		case GLUT_LEFT_BUTTON:
			camera.changeAngle(mouse.dx, mouse.dy);
			break;
		case GLUT_MIDDLE_BUTTON:
			camera.changePosition(0, 0, mouse.dy);
			break;
		case GLUT_RIGHT_BUTTON:
			camera.changePosition(mouse.dx, mouse.dy, 0);
			break;
		}
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case '\033':
		exit(0);
		break;
	case ' ':
		switch (Game::getState()) {
		case 0:
			Game::start();
		case 1:
			bird.flap();
			break;
		}
		break;
	case 'r':
		if (Game::getState() == 2) {
			Game::reset();
		}
		break;
	case 't':
		Pipe::toggleEnableTrans();
		break;
	case '1':
	case '2':
	case '3':
		Game::changeView(key);
		break;
	}
}

void idle() {
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	std::cout << "-------------キーボード操作-------------" << std::endl;
	std::cout << "スペースキー: ゲームスタート・羽ばたく" << std::endl;
	std::cout << "Rキー: ゲームオーバー後リトライ" << std::endl;
	std::cout << "Tキー: 透過の有無の切り替え" << std::endl;
	std::cout << "       (動作が重い場合に使用してください)" << std::endl;
	std::cout << "       (デフォルト: 透過有り)" << std::endl;
	std::cout << "Escキー: ゲームの終了" << std::endl;
	std::cout << "数字キー(1～3): 視点モード切り替え" << std::endl << std::endl;

	std::cout << "-------------視点モード一覧-------------" << std::endl;
	std::cout << "1: 固定視点モード(デフォルト)" << std::endl;
	std::cout << "   横スクロール風の視点です。" << std::endl;
	std::cout << "2: 追跡視点モード" << std::endl;
	std::cout << "   鳥を至近距離で追跡します。" << std::endl;
	std::cout << "3: 自由視点モード" << std::endl;
	std::cout << "   マウスで自由に視点変更できます。" << std::endl << std::endl;

	std::cout << "-------------自由視点モード-------------" << std::endl;
	std::cout << "マウスボタンを押しながらをドラッグすると" << std::endl;
	std::cout << "以下の操作ができます。" << std::endl;
	std::cout << "左ボタン: 回転" << std::endl;
	std::cout << "中ボタン: ズーム" << std::endl;
	std::cout << "右ボタン: 移動" << std::endl << std::endl;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// ウィンドウの設定
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("Flappy 3D Bird");

	// 描画初期設定
	init();
	Game::init();

	// イベントハンドラ設定
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutMouseFunc(mouse.pClick);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop();

	Game::cleanup();

	return 0;
}