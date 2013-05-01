#ifndef GLRENDERER_H
#define GLRENDERER_H

#include <QGLWidget>
#include "RenderingContext.h"

class GLRenderer : public QGLWidget
{
public:
    GLRenderer(QWidget *parent, ARDoor::RenderingContext *context) : QGLWidget(parent), context(context) {}

    void updateBackground(const cv::Mat &mat)
    {
        context->updateBackground(mat);
        update();
    }

protected:
    void initializeGL() { context->initialize(); }
    void resizeGL(int width, int height)
    {
        int side = qMin(width, height);
        glViewport((width - side) / 2, (height - side) / 2, side, side);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
    #ifdef QT_OPENGL_ES_1
        glOrthof(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
    #else
        glOrtho(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
    #endif
        glMatrixMode(GL_MODELVIEW);
    }
    void paintGL() { context->draw(); }

private:
    ARDoor::RenderingContext *context;
};

#endif // GLRENDERER_H
