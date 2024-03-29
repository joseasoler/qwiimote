/*
 * This file is part of QWiimote.
 *
 * QWiimote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QWiimote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QWiimote. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file wopengl.h
 *
 * Header file for the testing QGLWidget.
 */

#ifndef WOPENGL_H
#define WOPENGL_H

#include <QWidget>
#include <QGLWidget>
#include <QTimer>
#include <QMatrix4x4>

class WOpenGL : public QGLWidget
{
	Q_OBJECT
public:
	explicit WOpenGL(QWidget *parent = 0);
	void updateRotation(QMatrix4x4 new_rotation);

signals:

public slots:

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);

private:
	QMatrix4x4 rotation;

	QTimer update_timer;
};

#endif // WOPENGL_H
