#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CalibrationDialog.h"
#include "DebugHelper.h"
#include <iostream>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    context = new ARDoor::RenderingContext(&calibrator);

    glRenderer = new GLRenderer(this, context);
    ui->cameraContainer->addWidget(glRenderer, 1);

    // create image pipeline
    pipeline = new ARDoor::ImagePipeline(calibrator);

    testProcessor = new ARDoor::TestImageProcessor(calibrator);
    pipeline->registerProcessor(testProcessor);

    ARDoor::ImagePipeline::Configuration configuration;
    configuration.push_back("test");
    pipeline->setConfiguration(configuration);

    // setup image processor and camera
    imageProcessor = new CameraImageProcessor(glRenderer, pipeline);
    camera.setCaptureMode(QCamera::CaptureViewfinder);
    camera.setViewfinder(imageProcessor);

    // load calibration settings
    if (settings.contains("calibration/matrix/intrinsics/m00")) {
        cv::Mat intrinsics = cv::Mat(3, 3, CV_32F);
        cv::Mat distortion = cv::Mat(1, 5, CV_32F);

        settings.beginGroup("calibration/matrix/intrinsics");
        for (int i = 0; i < intrinsics.rows; ++i) {
            for (int j = 0; j < intrinsics.cols; ++j) {
                QString settingName = QString("m") + QString::number(i) + QString::number(j);
                intrinsics.at<float>(i, j) = settings.value(settingName).toFloat();
            }
        }
        settings.endGroup();

        settings.beginGroup("calibration/matrix/distortion");
        for (int i = 0; i < distortion.rows; ++i) {
            for (int j = 0; j < distortion.cols; ++j) {
                QString settingName = QString("m") + QString::number(i) + QString::number(j);
                distortion.at<float>(i, j) = settings.value(settingName).toFloat();
            }
        }
        settings.endGroup();

        DebugHelper::printMat<float>(intrinsics);
        DebugHelper::printMat<float>(distortion);

        calibrator.setIntrinsicsMatrix(intrinsics);
        calibrator.setDistortionCoeffs(distortion);
    }
}

MainWindow::~MainWindow()
{
    delete testProcessor;
    delete pipeline;
    delete imageProcessor;
    delete cameraWidget;
    delete ui;
}

void MainWindow::onCalibrationAction()
{
    cv::Size size = cv::Size(9, 6);
    std::vector<std::string> fileList;

    QList<QString> files = QFileDialog::getOpenFileNames(this, "Select image files for calibration");
    for (QList<QString>::iterator it = files.begin(); it != files.end(); ++it) {
        fileList.push_back(it->toStdString());
    }

    calibrator.addChessboardPoints(fileList, size);
}

void MainWindow::on_pushButton_clicked()
{
    if (camera.status() == QCamera::ActiveStatus) {
        camera.stop();
    } else {
        camera.start();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    CalibrationDialog dialog(&calibrator, this);
    dialog.exec();
}
