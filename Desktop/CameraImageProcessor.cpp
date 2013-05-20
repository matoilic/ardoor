#include "CameraImageProcessor.h"
#include "QtUtil.h"
#include <opencv2/core/core.hpp>

CameraImageProcessor::CameraImageProcessor(GLRenderer *renderer, ARDoor::ImagePipeline* pipeline)
{
    this->renderer = renderer;
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
    cv::Mat mat = QtUtil::convertToMat(frame);
    cv::cvtColor(mat, mat, CV_BGR2RGB);
    renderer->updateBackground(mat);
    return true;
}
