#ifndef CAMERAIMAGEPROCESSOR_H
#define CAMERAIMAGEPROCESSOR_H

#include "ImageWidget.h"
#include "ImagePipeline.h"
#include <QAbstractVideoSurface>

class CameraImageProcessor : public QAbstractVideoSurface
{
public:
    CameraImageProcessor(ImageWidget* widget, ARDoor::ImagePipeline* pipeline);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
    bool present(const QVideoFrame &frame);

private:
    ImageWidget* widget;
    ARDoor::ImagePipeline* pipeline;
};

#endif // CAMERAIMAGEPROCESSOR_H
