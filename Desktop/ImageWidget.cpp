
#include "ImageWidget.h"
#include "QtUtil.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent)
{
    imageLabel = new QLabel();
    layout = new QVBoxLayout();
    layout->addWidget(imageLabel);
    setLayout(layout);
}

ImageWidget::~ImageWidget()
{
    delete imageLabel;
    delete layout;
}

void ImageWidget::setMat(const cv::Mat &mat)
{
    setImage(QtUtil::convertToImage(mat));
}

void ImageWidget::setFrame(const QVideoFrame &frame)
{
    setImage(QtUtil::convertToImage(frame));
}

void ImageWidget::setImage(const QImage &image)
{
    int width = fmin(imageLabel->width(), image.width());
    int height = fmin(imageLabel->height(), image.height());
    QImage scaledImage = image.scaled(width, height, Qt::KeepAspectRatio);
    QPixmap pixmap = QPixmap::fromImage(scaledImage);
    imageLabel->setPixmap(pixmap);
}
