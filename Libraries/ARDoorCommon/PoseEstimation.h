//
//  PoseEstimation.h
//  ARDoor
//
//  Created by Mato Ilic on 19.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#ifndef __ARDoor__PoseEstimation__
#define __ARDoor__PoseEstimation__

#include <iostream>
#include "CameraCalibration.h"

namespace ARDoor {
class PoseEstimation {
    PoseEstimation(const cv::Mat &cameraMatrix, const cv::Mat &disortionCoefficients);
    virtual ~PoseEstimation();
    void projectObject(cv::Mat &img);
    
private:
    cv::Mat _cameraMatrix;
    cv::Mat _disortionCoefficients;
    CameraCalibration *_calibrator;
};
}

#endif /* defined(__ARDoor__PoseEstimation__) */
