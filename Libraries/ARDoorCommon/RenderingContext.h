#ifndef RENDERINGCONTEXT_H
#define RENDERINGCONTEXT_H

#include "CameraCalibration.h"
#include <vector>

namespace ARDoor {

class RenderingContext
{
public:
    RenderingContext(CameraCalibration *c);
    ~RenderingContext();

    void initialize();
    void draw();

    void updateBackground(const cv::Mat& frame);

private:
    void drawCameraFrame();
    void drawAugmentedScene();
    void drawCoordinateAxis();
    void drawCubeModel();

    void getPlanarSurface(std::vector<cv::Point2f>& imgP);
    void cvtPtoKpts(std::vector<cv::KeyPoint>& kpts, std::vector<cv::Point2f>& points);

private:
    bool               m_isTextureInitialized;
    unsigned int       m_backgroundTextureId;
    CameraCalibration  *m_calibration;
    cv::Mat            m_backgroundImage;

    cv::Mat perspMat;
};

}

#endif // RENDERINGCONTEXT_H
