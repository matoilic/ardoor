
#ifndef __ARDoor__PoseEstimation__
#define __ARDoor__PoseEstimation__

#include <iostream>
#include "CameraCalibration.h"

namespace ARDoor
{    
    class PoseEstimation
    {
        PoseEstimation(const cv::Mat &cameraMatrix, const cv::Mat &disortionCoefficients, cv::Size boardSize);
        virtual ~PoseEstimation();
        void projectObject(cv::Mat &img);
        
    private:
        cv::Mat _cameraMatrix;
        cv::Mat _disortionCoefficients;
        cv::Size _boardSize;
        cv::Mat _rotationMatrix;
        cv::Mat _rotationVector;
        cv::Mat _imagePoints;
        double _xView[9];
        CameraCalibration *_calibrator;
    };
}

#endif 
