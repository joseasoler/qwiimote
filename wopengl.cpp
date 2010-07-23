#include "wopengl.h"

WOpenGL::WOpenGL(QWidget *parent) : QGLWidget(parent)
{
	angle_x = 0;
	angle_y = 0;
	angle_z = 0;
}

void WOpenGL::initializeGL()
{
	qglClearColor(QColor(0, 0, 0));

	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void WOpenGL::UpdateAngles(qreal a_x, qreal a_y, qreal a_z)
{
	this->angle_x = a_x;
	this->angle_y = a_y;
	this->angle_z = a_z;
}

void WOpenGL::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -10.0);
	glRotatef(this->angle_x, 1.0, 0.0, 0.0);
	glRotatef(this->angle_y, 0.0, 1.0, 0.0);
	glRotatef(this->angle_z, 0.0, 0.0, 1.0);

	float size = 0.1;
	float vertex[8][3] = {
		{ size,  size * 0.75,  size * 4},
		{-size,  size * 0.75,  size * 4},
		{-size,  size * 0.75, -size * 4},
		{ size,  size * 0.75, -size * 4},
		{ size, -size * 0.75,  size * 4},
		{-size, -size * 0.75,  size * 4},
		{-size, -size * 0.75, -size * 4},
		{ size, -size * 0.75, -size * 4}
	};

	/* Draw the faces. */
	glPolygonMode(GL_FRONT, GL_FILL);
	glBegin(GL_QUADS);
		glColor3f(1.0, 1.0, 1.0);

		glVertex3fv(vertex[0]);
		glVertex3fv(vertex[1]);
		glVertex3fv(vertex[2]);
		glVertex3fv(vertex[3]);

		glVertex3fv(vertex[4]);
		glVertex3fv(vertex[5]);
		glVertex3fv(vertex[1]);
		glVertex3fv(vertex[0]);

		glVertex3fv(vertex[7]);
		glVertex3fv(vertex[4]);
		glVertex3fv(vertex[0]);
		glVertex3fv(vertex[3]);

		glVertex3fv(vertex[6]);
		glVertex3fv(vertex[7]);
		glVertex3fv(vertex[3]);
		glVertex3fv(vertex[2]);

		glVertex3fv(vertex[5]);
		glVertex3fv(vertex[6]);
		glVertex3fv(vertex[2]);
		glVertex3fv(vertex[1]);

		glVertex3fv(vertex[7]);
		glVertex3fv(vertex[6]);
		glVertex3fv(vertex[5]);
		glVertex3fv(vertex[4]);

	glEnd();

	for (int i = 0; i < 8; i++) {
		vertex[i][0] *= 1.001;
		vertex[i][1] *= 1.001;
		vertex[i][2] *= 1.001;
	}

	/* Draw the edges. */
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
		glColor3f(0.0, 0.0, 0.0);

		glVertex3fv(vertex[0]);
		glVertex3fv(vertex[1]);
		glVertex3fv(vertex[2]);
		glVertex3fv(vertex[3]);

		glVertex3fv(vertex[4]);
		glVertex3fv(vertex[5]);
		glVertex3fv(vertex[1]);
		glVertex3fv(vertex[0]);

		glVertex3fv(vertex[7]);
		glVertex3fv(vertex[4]);
		glVertex3fv(vertex[0]);
		glVertex3fv(vertex[3]);

		glVertex3fv(vertex[6]);
		glVertex3fv(vertex[7]);
		glVertex3fv(vertex[3]);
		glVertex3fv(vertex[2]);

		glVertex3fv(vertex[5]);
		glVertex3fv(vertex[6]);
		glVertex3fv(vertex[2]);
		glVertex3fv(vertex[1]);

		glVertex3fv(vertex[7]);
		glVertex3fv(vertex[6]);
		glVertex3fv(vertex[5]);
		glVertex3fv(vertex[4]);

	glEnd();
}

void WOpenGL::resizeGL(int width, int height)
{
	int side = qMin(width, height);
	glViewport((width - side) / 2, (height - side) / 2, side, side);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-0.5, +0.5, +0.5, -0.5, 4.0, 15.0);
	glMatrixMode(GL_MODELVIEW);
}
