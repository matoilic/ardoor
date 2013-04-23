#include "ImagePipeline.h"

namespace ARDoor {

ImagePipeline::ImagePipeline(const CameraCalibration &calibration)
    : calibration(calibration)
{
}

void ImagePipeline::processFrame(const cv::Mat& inputFrame, cv::Mat& outputFrame)
{
    cv::Mat currentInputFrame(inputFrame);

    for (Configuration::iterator it = configuration.begin(); it != configuration.end(); ++it)
    {
        std::string processorName = *it;
        if (processors.count(processorName) == 1)
        {
            ImageProcessor* processor = processors.at(processorName);
            processor->processFrame(currentInputFrame, outputFrame);
            outputFrame.copyTo(currentInputFrame);
        }
    }
}

void ImagePipeline::registerProcessor(ImageProcessor *processor)
{
    ProcessorMapEntry entry(processor->getName(), processor);
    processors.insert(entry);
}

void ImagePipeline::setConfiguration(const Configuration& configuration)
{
    this->configuration = configuration;
}

}
