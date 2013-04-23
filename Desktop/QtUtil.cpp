#include "QtUtil.h"
#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <QPoint>

cv::Mat QtUtil::convertToMat(const QImage &image)
{
    QImage rgb = image.convertToFormat(QImage::Format_RGB888);
    return cv::Mat(rgb.height(), rgb.width(), CV_8UC3, (void*) rgb.scanLine(0), rgb.bytesPerLine());
}

cv::Mat QtUtil::convertToMat(const QVideoFrame &frame)
{
    return convertToMat(convertToImage(frame));
}

QImage QtUtil::convertToImage(const cv::Mat &mat)
{
    // 8-bits unsigned, NO. OF CHANNELS=1
    if (mat.type() == CV_8UC1)
    {
        // Set the color table (used to translate colour indexes to qRgb values)
        QVector<QRgb> colorTable;
        for (int i = 0; i < 256; i++)
        {
            colorTable.push_back(qRgb(i, i, i));
        }

        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*) mat.data;

        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        img.setColorTable(colorTable);
        return img;
    }

    // 8-bits unsigned, NO. OF CHANNELS=3
    if (mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*) mat.data;

        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return img;
    }

    return QImage();
}

QImage QtUtil::convertToImage(const QVideoFrame &frame)
{
    QVideoFrame copy(frame);
    if (copy.map(QAbstractVideoBuffer::ReadOnly)) {
        QImage::Format format = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
        return QImage(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), format);
    }
    return QImage();
}
