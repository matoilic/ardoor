#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include "CameraCalibration.h"
#include "ImageWidget.h"

#include <QDialog>

namespace Ui {
class CalibrationDialog;
}

class CalibrationDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CalibrationDialog(QWidget *parent = 0);
    ~CalibrationDialog();

    void setCalibrator(ARDoor::CameraCalibration* calibrator);
    
private slots:
    void on_pushButton_clicked();

private:
    ARDoor::CameraCalibration* calibrator;

    Ui::CalibrationDialog *ui;
    ImageWidget* matWidget;
};

#endif // CALIBRATIONDIALOG_H
