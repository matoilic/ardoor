
#ifndef __ARDoor__ImageUtils__
#define __ARDoor__ImageUtils__

#include <iostream>
#include <opencv2/opencv.hpp>

namespace ARDoor
{
    class ImageUtils
    {
    public:
        static void convertToGray(const cv::Mat &orig, cv::Mat &gray);
    };
}

#endif 
