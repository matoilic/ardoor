#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "CameraCalibration.h"
#include <opencv2/core/core.hpp>

namespace ARDoor {

class ImageProcessor
{
public:
    ImageProcessor() {}
    virtual ~ImageProcessor() {}

    virtual std::string getName() = 0;
    virtual void processFrame(const cv::Mat& inputFrame, cv::Mat& outputFrame) = 0;
};

}

#endif // IMAGEPROCESSOR_H
