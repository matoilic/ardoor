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
    PoseEstimation(const cv::Mat &cameraMatrix, const cv::Mat &disortionCoefficients, cv::Size boardSize);
    virtual ~PoseEstimation();
    void projectObject(cv::Mat &img);
    
private:
    cv::Mat _cameraMatrix;
    cv::Mat _disortionCoefficients;
    cv::Size _boardSize;
    cv::Mat _rotationMatrix;
    cv::Mat _rotationVector;
    double _xView[9];
    CameraCalibration *_calibrator;
};
    
}

#endif /* defined(__ARDoor__PoseEstimation__) */
