#ifndef QTUTIL_H
#define QTUTIL_H

#include <opencv2/core/core.hpp>
#include <QImage>
#include <QVideoFrame>

class QtUtil
{
public:
    static cv::Mat convertToMat(const QImage& image);
    static cv::Mat convertToMat(const QVideoFrame& frame);
    static QImage convertToImage(const cv::Mat& mat);
    static QImage convertToImage(const QVideoFrame& frame);
};

#endif // QTUTIL_H
