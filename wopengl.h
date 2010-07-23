#ifndef WOPENGL_H
#define WOPENGL_H

#include <QWidget>
#include <QGLWidget>

class WOpenGL : public QGLWidget
{
	Q_OBJECT
public:
	explicit WOpenGL(QWidget *parent = 0);
	void updateAngles(qreal a_x, qreal a_y, qreal a_z);

signals:

public slots:

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);

private:
	qreal angle_x;
	qreal angle_y;
	qreal angle_z;
};

#endif // WOPENGL_H
