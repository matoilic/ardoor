
#ifndef __ARDoor__PatterExtractor__
#define __ARDoor__PatterExtractor__

#include <iostream>
#include <opencv2/opencv.hpp>
#include "Pattern.h"

namespace ARDoor
{
    
    class PatternExtractor
    {
    public:
        PatternExtractor(cv::Ptr<cv::FeatureDetector> detector, cv::Ptr<cv::DescriptorExtractor> extractor);
        void extract(const cv::Mat &img, Pattern &pattern);
        
    private:
        cv::Ptr<cv::FeatureDetector> featureDetector;
        cv::Ptr<cv::DescriptorExtractor> descriptorExtractor;
        
        bool extractFeatures(const cv::Mat &img, std::vector<cv::KeyPoint> &keypoints, cv::Mat &descriptors) const;
        void initializePattern(const cv::Mat &img, Pattern &pattern);
    };
    
}

#endif 
