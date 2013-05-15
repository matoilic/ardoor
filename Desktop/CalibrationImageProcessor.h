#ifndef CALIBRATIONIMAGEPROCESSOR_H
#define CALIBRATIONIMAGEPROCESSOR_H

#include "CameraCalibration.h"
#include "ImageWidget.h"
#include <ctime>
#include <QAbstractVideoSurface>

class CalibrationImageProcessor : public QAbstractVideoSurface
{
public:
    CalibrationImageProcessor(ARDoor::CameraCalibration* calibration, ImageWidget* widget);

    static cv::Size BOARD_SIZE;

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
    bool present(const QVideoFrame &frame);

    cv::Size getImageSize();

private:
    ARDoor::CameraCalibration* calibration;
    ImageWidget* widget;
    cv::Size imageSize;
    time_t prevTimestamp;
};

#endif // CALIBRATIONIMAGEPROCESSOR_H
