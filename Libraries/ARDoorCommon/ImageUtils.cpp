//
//  ImageUtils.cpp
//  ARDoor
//
//  Created by Mato Ilic on 08.05.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#include "ImageUtils.h"

namespace ARDoor
{
    /**
     * Converts the given original image to a gray scale image.
     * Expects orig to be BGR, BGRA or GRAY.
     */
    void ImageUtils::convertToGray(const cv::Mat &orig, cv::Mat &gray)
    {
        if (orig.channels()  == 3)
        {
            cv::cvtColor(orig, gray, CV_BGR2GRAY);
        }
        else if (orig.channels() == 4)
        {
            cv::cvtColor(orig, gray, CV_BGRA2GRAY);
        }
        else if (orig.channels() == 1)
        {
            gray = orig;
        }
    }
}