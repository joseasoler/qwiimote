#ifndef WOPENGL_H
#define WOPENGL_H

#include <QWidget>
#include <QGLWidget>

class WOpenGL : public QGLWidget
{
	Q_OBJECT
public:
	explicit WOpenGL(QWidget *parent = 0);

signals:

public slots:

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
};

#endif // WOPENGL_H
