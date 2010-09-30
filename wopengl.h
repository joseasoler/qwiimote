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
