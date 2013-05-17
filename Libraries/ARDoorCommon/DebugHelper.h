#ifndef DEBUGHELPER_H
#define DEBUGHELPER_H

#include <opencv2/core/core.hpp>

class DebugHelper
{
public:

    template<class T>
    static void printMat(const cv::Mat_<T>& mat)
    {
        for (int i = 0; i < mat.rows; ++i) {
            for (int j = 0; j < mat.cols; j++) {
                std::cout << mat.template at<T>(i, j) << "\t";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
};

#endif // DEBUGHELPER_H
