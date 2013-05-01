#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CalibrationDialog.h"
#include <iostream>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // setup camera widget
    //cameraWidget = new ImageWidget();
    //ui->cameraContainer->addWidget(cameraWidget, 1);

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
    CalibrationDialog dialog(this);
    dialog.exec();
}
