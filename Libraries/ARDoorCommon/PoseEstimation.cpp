//
//  PoseEstimation.cpp
//  ARDoor
//
//  Created by Mato Ilic on 19.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#include "PoseEstimation.h"

namespace ARDoor
{
    PoseEstimation::PoseEstimation(const cv::Mat &cameraMatrix, const cv::Mat &disortionCoefficients, cv::Size boardSize)
    {
        cameraMatrix.copyTo(_cameraMatrix);
        disortionCoefficients.copyTo(_disortionCoefficients);
        _boardSize = boardSize;
        _calibrator = new CameraCalibration();
        double _d[9] = {1, 0, 0, 0, -1, 0, 0, 0, -1}; //rotation: looking at -x axis
        _rotationMatrix = cv::Mat(3, 3, CV_64FC1, _d);
    }
    
    PoseEstimation::~PoseEstimation()
    {
        delete(_calibrator);
    }
    
    void PoseEstimation::projectObject(cv::Mat &img)
    {
        std::vector<cv::Point2f> imageCorners;
        std::vector<cv::Point3f> objectCorners;
        _calibrator->findChessboardPoints(img, _boardSize, imageCorners, objectCorners);
    }
}