#pragma once
#include "utils.h"

//-----------------------------------------------------------------------------
// �v�Z�p�̊֐�

// �x�N�g����\��
void showVectord(GLdouble v[]) {
	for (int i = 0; i < 3; i++) {
		std::cout << v[i] << std::endl;
	}
}

// �x�N�g���𐳋K��
void normalizeVectord(GLdouble v[]){
	double m = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	if (m > 0.0f) {
		m = 1.0f / m;
	}
	else{
		m = 0.0f;
	}

	v[0] *= m;
	v[1] *= m;
	v[2] *= m;
}

// �x�N�g���̊O��
void crossVectord(GLdouble src1[], GLdouble src2[], GLdouble out[]){
	out[0] = src1[1] * src2[2] - src1[2] * src2[1];
	out[1] = src1[2] * src2[0] - src1[0] * src2[2];
	out[2] = src1[0] * src2[1] - src1[1] * src2[0];
}

// �s���\��
void showMatrixd(GLdouble m[]) {
	for (int i = 0; i < 4; i++) {
		std::cout << m[i] << " " << m[i + 4] << " " << m[i + 8] << " " << m[i + 12] << std::endl;
	}
	std::cout << std::endl;
}

// �s���]�u
void transposeMatrixd(GLdouble src[], GLdouble out[]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			out[4 * i + j] = src[5 * (i + j) - (4 * i + j)];
		}
	}
}

// �s��ƃx�N�g���̐�
void mapLineard(GLdouble m[], GLdouble vi[], GLdouble vo[]) {
	for (int i = 0; i < 3; i++) {
		vo[i] = vi[0] * m[i] + vi[1] * m[i + 4] + vi[2] * m[i + 8];
	}
}

// ��]�s����v�Z
void rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z, GLdouble m[]) {
	double rad = toRadian(angle);
	double sin = std::sin(rad);
	double cos = std::cos(rad);
	double cos1 = 1 - cos;

	m[0] = x * x * cos1 + cos;
	m[1] = x * y * cos1 + z * sin;
	m[2] = z * x * cos1 - y * sin;
	m[3] = 0;
	m[4] = x * y * cos1 - z * sin;
	m[5] = y * y * cos1 + cos;
	m[6] = y * z * cos1 + x * sin;
	m[7] = 0;
	m[8] = z * x * cos1 + y * sin;
	m[9] = y * z * cos1 - x * sin;
	m[10] = z * z * cos1 + cos;
	m[11] = 0;
	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;
}

//-----------------------------------------------------------------------------
// Camera�N���X

// ���f���r���[�ϊ��s��
GLdouble Camera::matrix[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};
// ���f�����_�̐[��
GLdouble Camera::depth = 0;
// 1�s�N�Z��������̈ړ���(���s�ړ���)
GLdouble Camera::movePerPixel;
// 1�s�N�Z��������̈ړ���(��]��)
GLdouble Camera::rotatePerPixel;
// �E�B���h�E��
GLsizei Camera::width;
// �E�B���h�E����
GLsizei Camera::height;
// ����p�x
GLdouble Camera::angle;

// ������(�r���[�ϊ��s����v�Z�EgluLookAt�̑���)
void Camera::init(GLdouble x, GLdouble y, GLdouble z, GLdouble cx, GLdouble cy, GLdouble cz, GLdouble ux, GLdouble uy, GLdouble uz) {
	// �����̕���(Z��)
	double forward[3] = { cx - x, cy - y, cz - z };
	// �J�����̏����(Y��)
	double up[3] = { ux, uy, uz };
	// �J�����̉�����(X��)
	double side[3];
	// �J�����̈ʒu(��]�O)
	double rel[3] = { -x, -y, -z };
	// �J�����̈ʒu(��]��)
	double abs[3];

	// Z���𐳋K��
	normalizeVectord(forward);

	// X�����v�Z(Z����X���EY����X���𒼌�������)
	crossVectord(forward, up, side);
	normalizeVectord(side);

	// Y�����Čv�Z(Z����Y���𒼌�������)
	crossVectord(side, forward, up);

	// �l��ۑ�(���������͕������])
	Camera::matrix[0] = side[0]; Camera::matrix[1] = up[0]; Camera::matrix[2] = -forward[0];
	Camera::matrix[4] = side[1]; Camera::matrix[5] = up[1]; Camera::matrix[6] = -forward[1];
	Camera::matrix[8] = side[2]; Camera::matrix[9] = up[2]; Camera::matrix[10] = -forward[2];

	// �J�����ʒu�ɉ�]�s��𔽉f
	mapLineard(Camera::matrix, rel, abs);

	// �l��ۑ�
	Camera::matrix[12] = abs[0];
	Camera::matrix[13] = abs[1];
	Camera::matrix[14] = abs[2];

	// ��ʒ��S�Ƃ̋������v�Z
	Camera::setDepth();
	// 1�s�N�Z��������̈ړ��ʂ��v�Z
	Camera::calcPerPixel();
}

// ���_���Z�b�g
void Camera::set(int width, int height, double angle, double zNear, double zFar) {
	Camera::width = width;
	Camera::height = height;

	// �r���[�|�[�g�ϊ�
	glViewport(0, 0, Camera::width, Camera::height);

	// �������e�̐ݒ�
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(angle, (double)Camera::width / (double)Camera::height, zNear, zFar);
}

// �ۑ�����Ă���r���[�ϊ��z���K�p
void Camera::attachMatrix() {
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(Camera::matrix);
}

// ��ʒ��S(���z)�Ƃ̋�����ݒ�
void Camera::setDepth() {
	Camera::depth = -Camera::matrix[14];
}

// 1�s�N�Z��������̈ړ��ʂ��v�Z
void Camera::calcPerPixel() {
	// �r���[�ϊ��͖߂��Ȃ�
	GLdouble model[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	// ���e�ϊ�
	GLdouble proj[16];
	// �r���[�|�[�g�ϊ�
	GLint view[16];
	// ��O���̊p�̍��W
	GLdouble start0[3];
	GLdouble end0[3];
	// �����̊p�̍��W
	GLdouble start1[3];
	GLdouble end1[3];
	
	// ���e�ϊ��s��ƃr���[�|�[�g�ϊ��s����擾
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);

	// ��O�̊p
	gluUnProject(0, 0, 0, model, proj, view, &start0[0], &start0[1], &start0[2]);
	gluUnProject(Camera::width, Camera::height, 0, model, proj, view, &end0[0], &end0[1], &end0[2]);
	// �����̊p
	gluUnProject(0, 0, 1, model, proj, view, &start1[0], &start1[1], &start1[2]);
	gluUnProject(Camera::width, Camera::height, 1, model, proj, view, &end1[0], &end1[1], &end1[2]);

	// �p�̎�O->���̃x�N�g�����v�Z
	start1[0] -= start0[0];
	start1[1] -= start0[1];
	start1[2] -= start0[2];
	end1[0] -= end0[0];
	end1[1] -= end0[1];
	end1[2] -= end0[2];
	
	// Z�̒�����1��
	start1[0] /= start1[2];
	start1[1] /= start1[2];
	start1[2] = 1.0;
	end1[0] /= end1[2];
	end1[1] /= end1[2];
	end1[2] = 1.0;

	// Z��depth�̎��̉�ʂ̊p�̍��W���v�Z
	start0[0] += start1[0] * (Camera::depth - start0[2]);
	start0[1] += start1[1] * (Camera::depth - start0[2]);
	start0[2] = -Camera::depth;
	end0[0] += end1[0] * (Camera::depth - end0[2]);
	end0[1] += end1[1] * (Camera::depth - end0[2]);
	end0[2] = -Camera::depth;

	// ���s�ړ��p�̈ړ���
	Camera::movePerPixel = (start0[0] - end0[0]) / Camera::width;
	// ��]�p�̈ړ���
	Camera::rotatePerPixel = 10 * Camera::movePerPixel;
}

// �J�����̊p�x��ύX
void Camera::changeAngle(int dx, int dy) {
	if (dx || dy) {
		// �ړ���
		double ddx = dx * Camera::rotatePerPixel;
		double ddy = dy * Camera::rotatePerPixel;

		// �ړ�����
		GLdouble arc = std::sqrt(std::pow(ddx, 2) + std::pow(ddy, 2));
		// ��]�p�x
		GLdouble theta = toDegree(arc / Camera::depth);
		// ��]������(�������̍��W�n)
		GLdouble abs[3] = { ddy / arc, ddx / arc, 0.0};
		// ��]������(�r���[�ϊ���̍��W�n)
		GLdouble rel[3];
		// �r���[�ϊ��s��̓]�u�s��
		GLdouble rot[16];

		// �r���[�ϊ��s���]�u(�r���[�̉�]������)
		transposeMatrixd(Camera::matrix, rot);
		// �r���[�ϊ���̍��W�n�ł̎��������v�Z
		mapLineard(rot, abs, rel);
		// ��]
		glMatrixMode(GL_MODELVIEW);
		glRotated(theta, rel[0], rel[1], rel[2]);
		
		// �r���[�ϊ��s����L�^
		glGetDoublev(GL_MODELVIEW_MATRIX, Camera::matrix);

		// ��ʒ��S(0, 0, depth)����̃x�N�g�����v�Z
		GLdouble before[3] = {
			Camera::matrix[12],
			Camera::matrix[13],
			Camera::matrix[14] + Camera::depth
		};
		GLdouble after[3];

		// ��ʒ��S����̃x�N�g������]
		rotated(theta, abs[0], abs[1], abs[2], rot);
		mapLineard(rot, before, after);

		// ���s�ړ�
		Camera::matrix[12] += after[0] - before[0];
		Camera::matrix[13] += after[1] - before[1];
		Camera::matrix[14] += after[2] - before[2];

		// �r���[�ϊ��s����ēK�p
		glLoadMatrixd(Camera::matrix);

		glutPostRedisplay();
	}
}

// �J�����̈ʒu��ύX
void Camera::changePosition(int dx, int dy, int dz) {
	if (dx || dy || dz) {
		// ���s�ړ�
		Camera::matrix[12] += dx * Camera::movePerPixel;
		Camera::matrix[13] += dy * -Camera::movePerPixel;
		Camera::matrix[14] += dz * 0.05;

		// �r���[�ϊ��s����ēK�p
		Camera::attachMatrix();

		if (dz) {
			// ��ʒ��S�Ƃ̋������v�Z
			Camera::setDepth();
			// 1�s�N�Z��������̈ړ��ʂ��v�Z
			Camera::calcPerPixel();
		}

		glutPostRedisplay();
	}
}

// �J�����̈ʒu��ύX
void Camera::changePosition(double dx, double dy, double dz) {
	if (dx || dy || dz) {
		// ���s�ړ�
		Camera::matrix[12] += dx;
		Camera::matrix[13] += dy;
		Camera::matrix[14] += dz;

		// �r���[�ϊ��s����ēK�p
		Camera::attachMatrix();

		if (dz) {
			// ��ʒ��S�Ƃ̋������v�Z
			Camera::setDepth();
			// 1�s�N�Z��������̈ړ��ʂ��v�Z
			Camera::calcPerPixel();
		}

		glutPostRedisplay();
	}
}

//-----------------------------------------------------------------------------
// Mouse�N���X

int Mouse::x;
int Mouse::y;
int Mouse::x0;
int Mouse::y0;
int Mouse::dx;
int Mouse::dy;
int Mouse::button;
bool Mouse::down;

Mouse::Mouse() {
	down = false;
}

// �{�^��������
void Mouse::click(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		Mouse::x0 = x;
		Mouse::y0 = y;
		Mouse::dx = 0;
		Mouse::dy = 0;
		Mouse::button = button;
		Mouse::down = true;
	}
	else {
		Mouse::dx = x - Mouse::x;
		Mouse::dy = y - Mouse::y;
		Mouse::down = false;
	}
	Mouse::x = x;
	Mouse::y = y;
}

// �ړ���
void Mouse::motion(int x, int y) {
	if (Mouse::down) {
		Mouse::dx = x - Mouse::x;
		Mouse::dy = y - Mouse::y;
		Mouse::x = x;
		Mouse::y = y;
	}
}