#include "wopengl.h"

WOpenGL::WOpenGL(QWidget *parent) : QGLWidget(parent)
{
	this->rotation.setToIdentity();
	connect(&this->update_timer, SIGNAL(timeout()), this, SLOT(updateGL()));
	this->update_timer.start(100);
}

void WOpenGL::initializeGL()
{
	qglClearColor(QColor(0, 0, 0));

	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void WOpenGL::updateRotation(QMatrix4x4 new_rotation)
{
	this->rotation = new_rotation;
}

void WOpenGL::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -10.0);
	glRotatef(180, 0, 0, 1);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_LINES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(100.0, 0.0, 0.0);

		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 100.0, 0.0);

		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 100.0);
	glEnd();

	glPushMatrix();
	glMultMatrixd(rotation.constData());

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
