#include "wopengl.h"

WOpenGL::WOpenGL(QWidget *parent) :
    QGLWidget(parent)
{
}

void WOpenGL::initializeGL()
{
	qglClearColor(QColor(0, 0, 0));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void WOpenGL::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
}

void WOpenGL::resizeGL(int width, int height)
{
	int side = qMin(width, height);
	glViewport((width - side) / 2, (height - side) / 2, side, side);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
	glMatrixMode(GL_MODELVIEW);
}
