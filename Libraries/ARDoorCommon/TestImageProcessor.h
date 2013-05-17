#ifndef TESTIMAGEPROCESSOR_H
#define TESTIMAGEPROCESSOR_H

#include "ImageProcessor.h"

namespace ARDoor {

class TestImageProcessor : public ImageProcessor
{
public:
    TestImageProcessor(CameraCalibration& calibration);
    ~TestImageProcessor() {}

    std::string getName();
    void processFrame(const cv::Mat& inputFrame, cv::Mat& outputFrame);

private:
    CameraCalibration& calibration;
};

}

#endif // TESTIMAGEPROCESSOR_H
