
#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <opencv2/core/core.hpp>
#include <QPixmap>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QImage>
#include <QVideoFrame>

class ImageWidget : public QWidget {

    private:
        QLabel* imageLabel;
        QVBoxLayout* layout;
        
    public:
        ImageWidget(QWidget* parent = 0);
        ~ImageWidget();

        void setMat(const cv::Mat& mat);
        void setImage(const QImage& image);
        void setFrame(const QVideoFrame& frame);
};

#endif
