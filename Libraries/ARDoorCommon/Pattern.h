
#ifndef __ARDoor__Pattern__
#define __ARDoor__Pattern__

#include <iostream>
#include <opencv2/opencv.hpp>

namespace ARDoor
{
    struct Pattern
    {
        cv::Size size;
        cv::Mat frame;
        cv::Mat grayImage;
        
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        
        std::vector<cv::Point2f> points2d;
        std::vector<cv::Point3f> points3d;
    };
}

#endif
