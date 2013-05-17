#include "CalibrationImageProcessor.h"
#include "QtUtil.h"
#include <opencv2/core/core.hpp>

cv::Size CalibrationImageProcessor::BOARD_SIZE = cv::Size(9, 6);

CalibrationImageProcessor::CalibrationImageProcessor(ARDoor::CameraCalibration *calibration, ImageWidget *widget)
{
    this->calibration = calibration;
    this->widget = widget;
}

QList<QVideoFrame::PixelFormat> CalibrationImageProcessor::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);
    // Return the formats you will support
    return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB32;
}

bool CalibrationImageProcessor::present(const QVideoFrame &frame)
{
    cv::Mat mat = QtUtil::convertToMat(frame);
    cv::Mat grey;
    cv::cvtColor(mat, grey, CV_RGB2GRAY);

    std::vector<cv::Point2f> imageCorners;
    std::vector<cv::Point3f> objectCorners;

    time_t delay = 1;

    if (time(NULL) - prevTimestamp > delay) {
        if (calibration->findChessboardPoints(grey, BOARD_SIZE, imageCorners, objectCorners)) {
            std::cout << "Chessboard found" << std::endl;

            imageSize = mat.size();
            calibration->addPoints(imageCorners, objectCorners);
            //cv::drawChessboardCorners(mat, imageSize, imageCorners, true);
        }

        prevTimestamp = time(NULL);
    }

    widget->setFrame(frame);
    widget->repaint();

    return true;
}

cv::Size CalibrationImageProcessor::getImageSize()
{
    return imageSize;
}
