
#ifndef __ARDoor__CameraCalibration__
#define __ARDoor__CameraCalibration__

#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace ARDoor {
    
    class CameraCalibration
    {
        // the points in world coordinates
        std::vector< std::vector<cv::Point3f> > objectPoints;
        // the point positions in pixels
        std::vector< std::vector<cv::Point2f> > imagePoints;
        // output Matrices
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;
        // used in image undistortion
        cv::Mat mapX, mapY;
        bool mustInitUndistort;
        
    public:
        CameraCalibration();

        cv::Mat getIntrinsicsMatrix();

        cv::Mat getDistortionCoeffs();
        
        /**
         * Adds additional images to be used for calibration
         * @param fileList list of files to be loaded
         * @param boardSize number of rows and columns on the board
         * @return number of successfuly detected chess boards
         */
        int addChessboardPoints(const std::vector<std::string> &filelist, cv::Size &boardSize);
        
        /**
         * Adds points to be used for calibration
         * @param imageCorners point positions in world coordinates
         * @param objectCorners point positions in pixels
         */
        void addPoints(const std::vector<cv::Point2f> &imageCorners, std::vector<cv::Point3f> &objectCorners);
        
        /**
         * Performs the calibration.
         * @param imageSize the size of the image
         * @return the re-projection error
         */
        double calibrate(cv::Size &imageSize);
        
        bool findAndDrawChessboardPoints(const cv::Mat &image, cv::Size &boardSize, std::vector<cv::Point2f> &imageCorners, std::vector<cv::Point3f> &objectCorners);
        
        /**
         * Adds an additional image to be used for calibration
         * @param image the image to prcess
         * @param boardSize number of rows and columns on the board
         * @param imageCorners vector where the found image corners are written to
         * @param objectCorners vector where the found object corners are written to
         * @return true if the image could be processed, false otherwise
         */
        bool findChessboardPoints(const cv::Mat &image, cv::Size &boardSize, std::vector<cv::Point2f> &imageCorners, std::vector<cv::Point3f> &objectCorners);
        
        /**
         * Removes disortion from the given image
         * @param image
         * @return the undisorted image
         */
        cv::Mat remap(const cv::Mat &image);
        
    private:
        void printMat(const cv::Mat &mat, std::string name);
    };
    
}

#endif
