#include <ARDoorLib.h>
#include <opencv2/imgproc/imgproc.hpp>

cv::Mat* ARDoorLib::processCanny(cv::Mat& frame)
{
	cv::Mat* result = new cv::Mat();
	//cv::Mat gray;
	//cv::cvtColor(frame, gray, CV_RGBA2GRAY);
	cv::Canny(frame, *result, 30, 150);
	return result;
}
