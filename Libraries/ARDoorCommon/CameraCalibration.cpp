
#include "CameraCalibration.h"

namespace ARDoor {

CameraCalibration::CameraCalibration()
{
    mustInitUndistort = false;
}
    
int CameraCalibration::addChessboardPoints(const std::vector<std::string> &filelist, cv::Size &boardSize)
{
    std::vector<cv::Point2f> imageCorners;
    std::vector<cv::Point3f> objectCorners;
    
    cv::Mat image;
    int successes = 0;
    
    for (int i = 0; i < filelist.size(); i++)
    {
        image = cv::imread(filelist[i],0);
        
        if (findChessboardPoints(image, boardSize, imageCorners, objectCorners)) {
            addPoints(imageCorners, objectCorners);
            successes++;
        }
    }
    
    return successes;
}

void CameraCalibration::addPoints(const std::vector<cv::Point2f> &imageCorners, std::vector<cv::Point3f> &objectCorners)
{
    mustInitUndistort= true;
    imagePoints.push_back(imageCorners);
    objectPoints.push_back(objectCorners);
}

double CameraCalibration::calibrate(cv::Size &imageSize)
{
    mustInitUndistort= true;
    std::vector<cv::Mat> rotationVecs, translationVecs;
    
    return cv::calibrateCamera(
        objectPoints,
        imagePoints,
        imageSize,
        cameraMatrix,
        distCoeffs,
        rotationVecs,
        translationVecs
    );
}
    
bool CameraCalibration::findAndDrawChessboardPoints(const cv::Mat &image, cv::Size &boardSize, std::vector<cv::Point2f> &imageCorners, std::vector<cv::Point3f> &objectCorners)
{
    cv::Mat greyImage;
    if(image.type() == CV_8UC4)
    {
        cv::cvtColor(image, greyImage, CV_BGR2GRAY);
    }
    else
    {
        greyImage = image;
    }
    
    bool success = findChessboardPoints(greyImage, boardSize, imageCorners, objectCorners);
    
    cv::drawChessboardCorners(image, boardSize, imageCorners, success);
    
    return success;
}
    
bool CameraCalibration::findChessboardPoints(const cv::Mat &image, cv::Size &boardSize, std::vector<cv::Point2f> &imageCorners, std::vector<cv::Point3f> &objectCorners)
{
    for(int i = 0; i < boardSize.height; i++)
    {
        for(int j = 0; j < boardSize.width; j++)
        {
            objectCorners.push_back(cv::Point3f(i * 110, j * 110, 0.0f)); //110 = size of one square on the board
        }
    }
    
    bool success = cv::findChessboardCorners(image, boardSize, imageCorners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
    
    if(success) {
        cv::cornerSubPix(
            image,
            imageCorners,
            cv::Size(5,5),
            cv::Size(-1,-1),
            cv::TermCriteria(
                cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS,
                30,  // max number of iterations
                0.1  // min accuracy
            )
        );
    }
    
    objectCorners.resize(imageCorners.size(), objectCorners[0]);
    
    return imageCorners.size() == boardSize.area();
}

cv::Mat CameraCalibration::remap(const cv::Mat &image)
{
    cv::Mat undistorted;
    
    if(mustInitUndistort) { 
        cv::initUndistortRectifyMap(
            cameraMatrix,
            distCoeffs,
            cv::Mat(),
            cv::Mat(),
            image.size(),
            CV_32F,
            mapX,
            mapY           
        );
        
        printMat(cameraMatrix, "Camera Matrix");
        printMat(distCoeffs, "Disortion Coefficients");
        
        mustInitUndistort= false;
    }
    
    cv::remap(image, undistorted, mapX, mapY, cv::INTER_LINEAR);
    
    return undistorted;
}
    
void CameraCalibration::printMat(const cv::Mat &mat, std::string name)
{
    int r, c;
    
    std::cout << name << std::endl;
    
    for(r = 0; r < mat.rows; r++)
    {
        for(c = 0; c < mat.cols; c++)
        {
            std::cout << mat.row(r).col(c) << " ";
        }
        
        std::cout << std::endl;
    }
}

}