#include "CameraImageProcessor.h"
#include "QtUtil.h"
#include <opencv2/core/core.hpp>

CameraImageProcessor::CameraImageProcessor(ImageWidget* widget, ARDoor::ImagePipeline* pipeline)
{
    this->widget = widget;
    this->pipeline = pipeline;
}

QList<QVideoFrame::PixelFormat> CameraImageProcessor::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);
    // Return the formats you will support
    return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB32;
}

bool CameraImageProcessor::present(const QVideoFrame &frame)
{
    cv::Mat output;
    cv::Mat input = QtUtil::convertToMat(frame);

    pipeline->processFrame(input, output);
    widget->setMat(output);

    return true;
}
