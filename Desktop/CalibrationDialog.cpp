#include "CalibrationDialog.h"
#include "ui_CalibrationDialog.h"

#include <QFileDialog>
#include <QStandardItemModel>
#include <opencv2/highgui/highgui.hpp>

CalibrationDialog::CalibrationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalibrationDialog)
{
    ui->setupUi(this);

    matWidget = new ImageWidget(this);
    ui->mainContainer->addWidget(matWidget, 1);
}

CalibrationDialog::~CalibrationDialog()
{
    delete ui;
}

void CalibrationDialog::setCalibrator(ARDoor::CameraCalibration *calibrator)
{
    this->calibrator = calibrator;
}

void CalibrationDialog::on_pushButton_clicked()
{

    /*
    QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more files to open");
    QStandardItemModel* model = new QStandardItemModel(this);

    QStringList::Iterator it = files.begin();
    while (it != files.end()) {
        QStandardItem* item = new QStandardItem(QIcon(*it), "");
        model->appendRow(item);
        ++it;
    }

    ui->listView->setModel(model);
    */

    QList<QString> files = QFileDialog::getOpenFileNames(this, "Select file");

    for (QList<QString>::iterator it = files.begin(); it != files.end(); ++it) {
        QString file = *it;
        QImage image(file, "JPG");

        std::vector<cv::Point2f> imageCorners;
        std::vector<cv::Point3f> objectCorners;
        cv::Size size = cv::Size(9, 6);

        try {
            cv::Mat mat(image.height(), image.width(), CV_8UC4, image.bits(), image.bytesPerLine());
            if (calibrator->findChessboardPoints(mat, size, imageCorners, objectCorners))
            {
                calibrator->addPoints(imageCorners, objectCorners);
            }
            cv::drawChessboardCorners(mat, size, imageCorners, true);

            //cv::Canny(mat, mat, 30, 150, 3);
            matWidget->setMat(mat);
            matWidget->repaint();

        } catch (cv::Exception& e) {
            std::cout << file.toStdString() << std::endl;
            std::cout << e.what() << std::endl;
        }
    }
}
