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

};

#endif // WOPENGL_H
