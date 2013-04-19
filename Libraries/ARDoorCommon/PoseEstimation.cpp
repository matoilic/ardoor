//
//  PoseEstimation.cpp
//  ARDoor
//
//  Created by Mato Ilic on 19.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#include "PoseEstimation.h"

namespace ARDoor {
    PoseEstimation::PoseEstimation(const cv::Mat &cameraMatrix, const cv::Mat &disortionCoefficients)
    {
        cameraMatrix.copyTo(_cameraMatrix);
        disortionCoefficients.copyTo(_disortionCoefficients);
        _calibrator = new CameraCalibration();
    }
    
    PoseEstimation::~PoseEstimation()
    {
        delete(_calibrator);
    }
    
    void PoseEstimation::projectObject(cv::Mat &img)
    {
        
    }
}