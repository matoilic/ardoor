#include <opencv/cv.h>

class ARDoorLib
{
public:
	static cv::Mat* processCanny(cv::Mat& frame);

	static void onSurfaceCreated();
	static void onSurfaceChanged(int width, int height);
	static void onDrawFrame();
};
