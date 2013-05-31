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
        glViewport(0, 0, width, height);
    }
    void paintGL() { context->draw(); }

private:
    ARDoor::RenderingContext *context;
};

#endif // GLRENDERER_H
