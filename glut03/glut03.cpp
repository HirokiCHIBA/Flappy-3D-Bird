#define _USE_MATH_DEFINES
#include "utils.h"
#include "flappy.h"

Game game;
Pipe pipe;
Floor ffloor;
Bird bird;
Mouse mouse;
Camera camera;

// �`�揉���ݒ�
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
	std::cout << "-------------�L�[�{�[�h����-------------" << std::endl;
	std::cout << "�X�y�[�X�L�[: �Q�[���X�^�[�g�E�H�΂���" << std::endl;
	std::cout << "R�L�[: �Q�[���I�[�o�[�ナ�g���C" << std::endl;
	std::cout << "T�L�[: ���߂̗L���̐؂�ւ�" << std::endl;
	std::cout << "       (���삪�d���ꍇ�Ɏg�p���Ă�������)" << std::endl;
	std::cout << "       (�f�t�H���g: ���ߗL��)" << std::endl;
	std::cout << "Esc�L�[: �Q�[���̏I��" << std::endl;
	std::cout << "�����L�[(1�`3): ���_���[�h�؂�ւ�" << std::endl << std::endl;

	std::cout << "-------------���_���[�h�ꗗ-------------" << std::endl;
	std::cout << "1: �Œ莋�_���[�h(�f�t�H���g)" << std::endl;
	std::cout << "   ���X�N���[�����̎��_�ł��B" << std::endl;
	std::cout << "2: �ǐՎ��_���[�h" << std::endl;
	std::cout << "   �������ߋ����ŒǐՂ��܂��B" << std::endl;
	std::cout << "3: ���R���_���[�h" << std::endl;
	std::cout << "   �}�E�X�Ŏ��R�Ɏ��_�ύX�ł��܂��B" << std::endl << std::endl;

	std::cout << "-------------���R���_���[�h-------------" << std::endl;
	std::cout << "�}�E�X�{�^���������Ȃ�����h���b�O�����" << std::endl;
	std::cout << "�ȉ��̑��삪�ł��܂��B" << std::endl;
	std::cout << "���{�^��: ��]" << std::endl;
	std::cout << "���{�^��: �Y�[��" << std::endl;
	std::cout << "�E�{�^��: �ړ�" << std::endl << std::endl;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// �E�B���h�E�̐ݒ�
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("Flappy 3D Bird");

	// �`�揉���ݒ�
	init();
	Game::init();

	// �C�x���g�n���h���ݒ�
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