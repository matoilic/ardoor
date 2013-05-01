#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "CameraCalibration.h"
#include "ImagePipeline.h"
#include "TestImageProcessor.h"
#include "CameraImageProcessor.h"
#include "ImageWidget.h"
#include "GLRenderer.h"

#include <QMainWindow>
#include <QCamera>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onCalibrationAction();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    ARDoor::CameraCalibration calibrator;
    ARDoor::RenderingContext* context;
    ARDoor::ImagePipeline* pipeline;
    ARDoor::TestImageProcessor* testProcessor;

    Ui::MainWindow *ui;
    ImageWidget* cameraWidget;
    GLRenderer* glRenderer;

    CameraImageProcessor* imageProcessor;
    QCamera camera;
};

#endif // MAINWINDOW_H
