#ifndef IMAGEPIPELINE_H
#define IMAGEPIPELINE_H

#include "CameraCalibration.h"
#include "ImageProcessor.h"
#include <opencv2/core/core.hpp>
#include <list>
#include <map>

namespace ARDoor {


class ImagePipeline
{
public:
    typedef std::list<std::string> Configuration;

    ImagePipeline(const CameraCalibration& calibration);

    void processFrame(const cv::Mat& inputFrame, cv::Mat& outputFrame);
    void registerProcessor(ImageProcessor* processor);
    void setConfiguration(const Configuration& configuration);

protected:
    const CameraCalibration& calibration;

private:
    typedef std::map<std::string, ImageProcessor*> ProcessorMap;
    typedef ProcessorMap::value_type ProcessorMapEntry;

    ProcessorMap processors;
    Configuration configuration;
};

}

#endif // IMAGEPIPELINE_H
