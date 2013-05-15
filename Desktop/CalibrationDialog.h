#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include "CameraCalibration.h"
#include "ImageWidget.h"
#include "CalibrationImageProcessor.h"
#include <QDialog>
#include <QCamera>
#include <QSettings>

namespace Ui {
class CalibrationDialog;
}

class CalibrationDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CalibrationDialog(ARDoor::CameraCalibration *calibrator, QWidget *parent = 0);
    ~CalibrationDialog();
    
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::CalibrationDialog *ui;

    ARDoor::CameraCalibration* calibrator;
    ImageWidget* matWidget;
    CalibrationImageProcessor* imageProcessor;
    QCamera camera;
    QSettings settings;
};

#endif // CALIBRATIONDIALOG_H
