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
    void resizeGL(int width, int height) { std::cout << "resize" << std::endl; std::cout.flush(); context->resize(width, height); }

protected:
    void initializeGL() { QGLWidget::initializeGL(); context->initialize(); }
    void paintGL() { context->draw(); }

private:
    ARDoor::RenderingContext *context;
};

#endif // GLRENDERER_H
