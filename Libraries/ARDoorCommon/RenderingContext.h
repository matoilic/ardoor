#ifndef RENDERINGCONTEXT_H
#define RENDERINGCONTEXT_H

#include "CameraCalibration.h"
#include <vector>

namespace ARDoor {

class RenderingContext
{
public:
    RenderingContext(CameraCalibration *c);

    void initialize();
    void draw();

    void updateBackground(const cv::Mat& frame);

private:
    void drawCameraFrame();
    void drawAugmentedScene();
    void drawCoordinateAxis();
    void drawCubeModel();

    void buildProjectionMatrix(CameraCalibration* calibration, int screen_width, int screen_height, cv::Mat& projectionMatrix);
    void detectChessboard();

private:
    bool               m_isTextureInitialized;
    unsigned int       m_backgroundTextureId;
    CameraCalibration  *m_calibration;
    cv::Mat            m_backgroundImage;

    bool isPatternPresent;
    cv::Mat objectPosition;
};

}

#endif // RENDERINGCONTEXT_H
