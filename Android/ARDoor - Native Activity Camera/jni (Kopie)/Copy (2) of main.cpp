#include <jni.h>
#include <android_native_app_glue.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "Logging.h"

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
extern "C" {
	void android_main(struct android_app* state) {
		// Make sure glue isn't stripped.
		app_dummy();

		cv::VideoCapture cap(CV_CAP_ANDROID + 0);
		cap.set(CV_CAP_PROP_FRAME_WIDTH, 800);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

		cv::Mat frame;

		while (cap.isOpened()) {
			cap.read(frame);
			if (!frame.empty()) {
				LOGI("frame received");
			}
		}
	}
}
