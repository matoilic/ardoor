#include "TestImageProcessor.h"
#include <opencv2/core/core.hpp>

namespace ARDoor {

TestImageProcessor::TestImageProcessor(CameraCalibration& calibration)
    : calibration(calibration)
{
}

std::string TestImageProcessor::getName() {
    return "test";
}

void TestImageProcessor::processFrame(const cv::Mat& inputFrame, cv::Mat& outputFrame)
{
    inputFrame.copyTo(outputFrame);

    //cv::Mat grey;
    //cv::cvtColor(inputFrame, grey, CV_RGB2GRAY);
    //cv::Canny(outputFrame, outputFrame, 50, 150);

    //std::vector<cv::Point2f> imageCorners;
    //std::vector<cv::Point3f> objectCorners;
    //cv::Size size = cv::Size(9, 6);

    //if (calibration.findChessboardPoints(grey, size, imageCorners, objectCorners))
    //{
    //    cv::drawChessboardCorners(outputFrame, size, imageCorners, true);
    //}
}

}
