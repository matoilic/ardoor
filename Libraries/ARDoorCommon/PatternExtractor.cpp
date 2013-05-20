
#include "PatternExtractor.h"
#include "ImageUtils.h"

namespace ARDoor
{
    PatternExtractor::PatternExtractor(cv::Ptr<cv::FeatureDetector> detector, cv::Ptr<cv::DescriptorExtractor> extractor)
    {
        featureDetector = detector;
        descriptorExtractor = extractor;
    }
    
    void PatternExtractor::extract(const cv::Mat &img, Pattern &pattern)
    {
        initializePattern(img, pattern);
        extractFeatures(img, pattern.keypoints, pattern.descriptors);
    }
    
    bool PatternExtractor::extractFeatures(const cv::Mat &img, std::vector<cv::KeyPoint> &keypoints, cv::Mat &descriptors) const
    {
        assert(!img.empty());
        assert(img.channels() == 1);
        
        featureDetector->detect(img, keypoints);        
        if (keypoints.empty())
        {
            return false;
        }
        
        descriptorExtractor->compute(img, keypoints, descriptors);
        if (keypoints.empty())
        {
            return false;
        }
        
        return true;
    }
    
    void PatternExtractor::initializePattern(const cv::Mat &img, Pattern &pattern)
    {
        pattern.size = cv::Size(img.cols, img.rows);
        pattern.frame = img.clone();
        ImageUtils::convertToGray(img, pattern.grayImage);
        
        pattern.points2d.resize(4);
        pattern.points3d.resize(4);
        
        const float width = img.cols;
        const float height = img.rows;
        
        const float maxSize = std::max(width, height);
        const float unitWidth = width / maxSize;
        const float unitHeight = height / maxSize;
        
        pattern.points2d[0] = cv::Point2f(0, 0);
        pattern.points2d[1] = cv::Point2f(width, 0);
        pattern.points2d[2] = cv::Point2f(width, height);
        pattern.points2d[3] = cv::Point2f(0, height);
        
        pattern.points3d[0] = cv::Point3f(-unitWidth, -unitHeight, 0);
        pattern.points3d[1] = cv::Point3f( unitWidth, -unitHeight, 0);
        pattern.points3d[2] = cv::Point3f( unitWidth,  unitHeight, 0);
        pattern.points3d[3] = cv::Point3f(-unitWidth,  unitHeight, 0);
    }
}
