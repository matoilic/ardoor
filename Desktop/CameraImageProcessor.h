#ifndef CAMERAIMAGEPROCESSOR_H
#define CAMERAIMAGEPROCESSOR_H

#include "GLRenderer.h"
#include "ImagePipeline.h"
#include <QAbstractVideoSurface>

class CameraImageProcessor : public QAbstractVideoSurface
{
public:
    CameraImageProcessor(GLRenderer *renderer, ARDoor::ImagePipeline* pipeline);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
    bool present(const QVideoFrame &frame);

private:
    GLRenderer *renderer;
    ARDoor::ImagePipeline* pipeline;
};

#endif // CAMERAIMAGEPROCESSOR_H
