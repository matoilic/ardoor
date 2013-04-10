#include <jni.h>
#include <opencv/cv.h>

#include <Logging.h>
#include <ARDoorLib.h>

extern "C" {

JNIEXPORT jlong JNICALL Java_ch_bfh_cpvr_ardoor_ARDoorLib_nativeCanny
(JNIEnv * jenv, jclass, jlong imageGray)
{
	cv::Mat& mGray = *((cv::Mat*) imageGray);

    try
    {
    	cv::Mat* mResult = ARDoorLib::processCanny(mGray);
    	return (jlong) mResult;
    }
    catch (cv::Exception& e)
    {
        LOGD("nativeCanny caught cv::Exception: %s", e.what());
        jclass je = jenv->FindClass("org/opencv/core/CvException");
        if (!je)
            je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, e.what());
    }
    catch (...)
    {
        LOGD("nativeCanny caught unknown exception");
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code");
    }

    return 0;
}

}
