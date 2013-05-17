#include "CalibrationDialog.h"
#include "ui_CalibrationDialog.h"
#include "CalibrationImageProcessor.h"
#include "DebugHelper.h"

#include <QCamera>

CalibrationDialog::CalibrationDialog(ARDoor::CameraCalibration *calibrator, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalibrationDialog)
{
    ui->setupUi(this);

    this->calibrator = calibrator;

    matWidget = new ImageWidget(this);
    ui->mainContainer->addWidget(matWidget, 1);

    // setup image processor and camera
    imageProcessor = new CalibrationImageProcessor(calibrator, matWidget);
    camera.setCaptureMode(QCamera::CaptureViewfinder);
    camera.setViewfinder(imageProcessor);
}

CalibrationDialog::~CalibrationDialog()
{
    delete ui;
    delete matWidget;
    delete imageProcessor;
}

void CalibrationDialog::on_pushButton_clicked()
{
    if (camera.status() == QCamera::ActiveStatus) {
        camera.stop();
    } else {
        camera.start();
    }
}

void CalibrationDialog::on_pushButton_2_clicked()
{
    cv::Size size = imageProcessor->getImageSize();
    calibrator->calibrate(size);

    cv::Mat_<float> intrinsics = calibrator->getIntrinsicsMatrix();
    cv::Mat_<float> distortion = calibrator->getDistortionCoeffs();

    DebugHelper::printMat<float>(intrinsics);
    DebugHelper::printMat<float>(distortion);


    settings.beginGroup("calibration/matrix/intrinsics");
    for (int i = 0; i < intrinsics.rows; ++i) {
        for (int j = 0; j < intrinsics.cols; ++j) {
            QString settingName = QString("m") + QString::number(i) + QString::number(j);
            settings.setValue(settingName, intrinsics.at<float>(i, j));
        }
    }
    settings.endGroup();

    std::cout.flush();

    settings.beginGroup("calibration/matrix/distortion");
    for (int i = 0; i < distortion.rows; ++i) {
        for (int j = 0; j < distortion.cols; ++j) {
            QString settingName = QString("m") + QString::number(i) + QString::number(j);
            settings.setValue(settingName, distortion.at<float>(i, j));
        }
    }
    settings.endGroup();
}
