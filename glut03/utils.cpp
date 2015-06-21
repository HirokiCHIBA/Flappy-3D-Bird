#pragma once
#include "utils.h"

//-----------------------------------------------------------------------------
// 計算用の関数

// ベクトルを表示
void showVectord(GLdouble v[]) {
	for (int i = 0; i < 3; i++) {
		std::cout << v[i] << std::endl;
	}
}

// ベクトルを正規化
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

// ベクトルの外積
void crossVectord(GLdouble src1[], GLdouble src2[], GLdouble out[]){
	out[0] = src1[1] * src2[2] - src1[2] * src2[1];
	out[1] = src1[2] * src2[0] - src1[0] * src2[2];
	out[2] = src1[0] * src2[1] - src1[1] * src2[0];
}

// 行列を表示
void showMatrixd(GLdouble m[]) {
	for (int i = 0; i < 4; i++) {
		std::cout << m[i] << " " << m[i + 4] << " " << m[i + 8] << " " << m[i + 12] << std::endl;
	}
	std::cout << std::endl;
}

// 行列を転置
void transposeMatrixd(GLdouble src[], GLdouble out[]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			out[4 * i + j] = src[5 * (i + j) - (4 * i + j)];
		}
	}
}

// 行列とベクトルの積
void mapLineard(GLdouble m[], GLdouble vi[], GLdouble vo[]) {
	for (int i = 0; i < 3; i++) {
		vo[i] = vi[0] * m[i] + vi[1] * m[i + 4] + vi[2] * m[i + 8];
	}
}

// 回転行列を計算
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
// Cameraクラス

// モデルビュー変換行列
GLdouble Camera::matrix[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};
// モデル原点の深さ
GLdouble Camera::depth = 0;
// 1ピクセル当たりの移動量(平行移動時)
GLdouble Camera::movePerPixel;
// 1ピクセル当たりの移動量(回転時)
GLdouble Camera::rotatePerPixel;
// ウィンドウ幅
GLsizei Camera::width;
// ウィンドウ高さ
GLsizei Camera::height;
// 視野角度
GLdouble Camera::angle;

// 初期化(ビュー変換行列を計算・gluLookAtの代わり)
void Camera::init(GLdouble x, GLdouble y, GLdouble z, GLdouble cx, GLdouble cy, GLdouble cz, GLdouble ux, GLdouble uy, GLdouble uz) {
	// 視線の方向(Z軸)
	double forward[3] = { cx - x, cy - y, cz - z };
	// カメラの上方向(Y軸)
	double up[3] = { ux, uy, uz };
	// カメラの横方向(X軸)
	double side[3];
	// カメラの位置(回転前)
	double rel[3] = { -x, -y, -z };
	// カメラの位置(回転後)
	double abs[3];

	// Z軸を正規化
	normalizeVectord(forward);

	// X軸を計算(Z軸とX軸・Y軸とX軸を直交させる)
	crossVectord(forward, up, side);
	normalizeVectord(side);

	// Y軸を再計算(Z軸とY軸を直交させる)
	crossVectord(side, forward, up);

	// 値を保存(視線方向は符号反転)
	Camera::matrix[0] = side[0]; Camera::matrix[1] = up[0]; Camera::matrix[2] = -forward[0];
	Camera::matrix[4] = side[1]; Camera::matrix[5] = up[1]; Camera::matrix[6] = -forward[1];
	Camera::matrix[8] = side[2]; Camera::matrix[9] = up[2]; Camera::matrix[10] = -forward[2];

	// カメラ位置に回転行列を反映
	mapLineard(Camera::matrix, rel, abs);

	// 値を保存
	Camera::matrix[12] = abs[0];
	Camera::matrix[13] = abs[1];
	Camera::matrix[14] = abs[2];

	// 画面中心との距離を計算
	Camera::setDepth();
	// 1ピクセルあたりの移動量を計算
	Camera::calcPerPixel();
}

// 視点をセット
void Camera::set(int width, int height, double angle, double zNear, double zFar) {
	Camera::width = width;
	Camera::height = height;

	// ビューポート変換
	glViewport(0, 0, Camera::width, Camera::height);

	// 透視投影の設定
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(angle, (double)Camera::width / (double)Camera::height, zNear, zFar);
}

// 保存されているビュー変換配列を適用
void Camera::attachMatrix() {
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(Camera::matrix);
}

// 画面中心(仮想)との距離を設定
void Camera::setDepth() {
	Camera::depth = -Camera::matrix[14];
}

// 1ピクセルあたりの移動量を計算
void Camera::calcPerPixel() {
	// ビュー変換は戻さない
	GLdouble model[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	// 投影変換
	GLdouble proj[16];
	// ビューポート変換
	GLint view[16];
	// 手前側の角の座標
	GLdouble start0[3];
	GLdouble end0[3];
	// 奥側の角の座標
	GLdouble start1[3];
	GLdouble end1[3];
	
	// 投影変換行列とビューポート変換行列を取得
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);

	// 手前の角
	gluUnProject(0, 0, 0, model, proj, view, &start0[0], &start0[1], &start0[2]);
	gluUnProject(Camera::width, Camera::height, 0, model, proj, view, &end0[0], &end0[1], &end0[2]);
	// 奥側の角
	gluUnProject(0, 0, 1, model, proj, view, &start1[0], &start1[1], &start1[2]);
	gluUnProject(Camera::width, Camera::height, 1, model, proj, view, &end1[0], &end1[1], &end1[2]);

	// 角の手前->奥のベクトルを計算
	start1[0] -= start0[0];
	start1[1] -= start0[1];
	start1[2] -= start0[2];
	end1[0] -= end0[0];
	end1[1] -= end0[1];
	end1[2] -= end0[2];
	
	// Zの長さを1に
	start1[0] /= start1[2];
	start1[1] /= start1[2];
	start1[2] = 1.0;
	end1[0] /= end1[2];
	end1[1] /= end1[2];
	end1[2] = 1.0;

	// Zがdepthの時の画面の角の座標を計算
	start0[0] += start1[0] * (Camera::depth - start0[2]);
	start0[1] += start1[1] * (Camera::depth - start0[2]);
	start0[2] = -Camera::depth;
	end0[0] += end1[0] * (Camera::depth - end0[2]);
	end0[1] += end1[1] * (Camera::depth - end0[2]);
	end0[2] = -Camera::depth;

	// 平行移動用の移動量
	Camera::movePerPixel = (start0[0] - end0[0]) / Camera::width;
	// 回転用の移動量
	Camera::rotatePerPixel = 10 * Camera::movePerPixel;
}

// カメラの角度を変更
void Camera::changeAngle(int dx, int dy) {
	if (dx || dy) {
		// 移動量
		double ddx = dx * Camera::rotatePerPixel;
		double ddy = dy * Camera::rotatePerPixel;

		// 移動距離
		GLdouble arc = std::sqrt(std::pow(ddx, 2) + std::pow(ddy, 2));
		// 回転角度
		GLdouble theta = toDegree(arc / Camera::depth);
		// 回転軸方向(見かけの座標系)
		GLdouble abs[3] = { ddy / arc, ddx / arc, 0.0};
		// 回転軸方向(ビュー変換後の座標系)
		GLdouble rel[3];
		// ビュー変換行列の転置行列
		GLdouble rot[16];

		// ビュー変換行列を転置(ビューの回転を解除)
		transposeMatrixd(Camera::matrix, rot);
		// ビュー変換後の座標系での軸方向を計算
		mapLineard(rot, abs, rel);
		// 回転
		glMatrixMode(GL_MODELVIEW);
		glRotated(theta, rel[0], rel[1], rel[2]);
		
		// ビュー変換行列を記録
		glGetDoublev(GL_MODELVIEW_MATRIX, Camera::matrix);

		// 画面中心(0, 0, depth)からのベクトルを計算
		GLdouble before[3] = {
			Camera::matrix[12],
			Camera::matrix[13],
			Camera::matrix[14] + Camera::depth
		};
		GLdouble after[3];

		// 画面中心からのベクトルを回転
		rotated(theta, abs[0], abs[1], abs[2], rot);
		mapLineard(rot, before, after);

		// 平行移動
		Camera::matrix[12] += after[0] - before[0];
		Camera::matrix[13] += after[1] - before[1];
		Camera::matrix[14] += after[2] - before[2];

		// ビュー変換行列を再適用
		glLoadMatrixd(Camera::matrix);

		glutPostRedisplay();
	}
}

// カメラの位置を変更
void Camera::changePosition(int dx, int dy, int dz) {
	if (dx || dy || dz) {
		// 平行移動
		Camera::matrix[12] += dx * Camera::movePerPixel;
		Camera::matrix[13] += dy * -Camera::movePerPixel;
		Camera::matrix[14] += dz * 0.05;

		// ビュー変換行列を再適用
		Camera::attachMatrix();

		if (dz) {
			// 画面中心との距離を計算
			Camera::setDepth();
			// 1ピクセルあたりの移動量を計算
			Camera::calcPerPixel();
		}

		glutPostRedisplay();
	}
}

// カメラの位置を変更
void Camera::changePosition(double dx, double dy, double dz) {
	if (dx || dy || dz) {
		// 平行移動
		Camera::matrix[12] += dx;
		Camera::matrix[13] += dy;
		Camera::matrix[14] += dz;

		// ビュー変換行列を再適用
		Camera::attachMatrix();

		if (dz) {
			// 画面中心との距離を計算
			Camera::setDepth();
			// 1ピクセルあたりの移動量を計算
			Camera::calcPerPixel();
		}

		glutPostRedisplay();
	}
}

//-----------------------------------------------------------------------------
// Mouseクラス

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

// ボタン押下時
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

// 移動時
void Mouse::motion(int x, int y) {
	if (Mouse::down) {
		Mouse::dx = x - Mouse::x;
		Mouse::dy = y - Mouse::y;
		Mouse::x = x;
		Mouse::y = y;
	}
}