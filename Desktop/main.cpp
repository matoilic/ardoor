#include <opencv2/opencv.hpp>
#include <QApplication>
#include "MainWindow.h"
#include "Pattern.h"
#include "PatternExtractor.h"
#include "ImageUtils.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("BFH");
    QCoreApplication::setOrganizationDomain("bfh.ch");
    QCoreApplication::setApplicationName("ARDoor");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    /*ARDoor::Pattern p;
    ARDoor::PatternExtractor *pe = new ARDoor::PatternExtractor (
        new cv::ORB(1000),
        new cv::FREAK(false, false)
    );

    cv::Mat img = cv::imread("/Users/matoilic/Desktop/calib-checkerboard.png");
    ARDoor::ImageUtils::convertToGray(img, img);
    pe->extract(img, p);
    delete pe;*/

    return a.exec();
}
