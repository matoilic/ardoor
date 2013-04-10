#include "ConcreteApp.h"
#include "Logging.h"

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

GLuint texture;
cv::VideoCapture capture;
cv::Mat rgbFrame;
cv::Mat inframe;
cv::Mat outframe;

GLfloat vertices[] = {
	  -1.0f, -1.0f, 0.0f, // V1 - bottom left
	  -1.0f,  1.0f, 0.0f, // V2 - top left
	   1.0f, -1.0f, 0.0f, // V3 - bottom right
	   1.0f,  1.0f, 0.0f // V4 - top right
};

GLfloat textures[8];


void ConcreteApp::OnInitialize()
{
	LOGD("OnInitialize()");
}

void ConcreteApp::OnInitDisplay()
{
	LOGD("OnInitDisplay()");

	int frameWidth = GetFrameWidth();
	int frameHeight = GetFrameHeight();

	LOGI("Frame size: %dx%d", frameWidth, frameHeight);

	capture.open(CV_CAP_ANDROID + 0);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, frameHeight);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, frameWidth);

	glViewport(0, 0, frameWidth,frameHeight);
	int screenWidth = frameWidth;
	int screenHeight = frameHeight;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = screenWidth / screenHeight;
	float bt = (float) tan(45 / 2);
	float lr = bt * aspect;
	glFrustumf(-lr * 0.1f, lr * 0.1f, -bt * 0.1f, bt * 0.1f, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepthf(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	textures[0] = 0.0f;
	textures[1] = (float) (frameWidth * 1.0 / screenHeight * 1.0) / (frameHeight * 1.0 / screenWidth * 1.0);
	textures[2] = 1.0f;
	textures[3] = (float) (frameWidth * 1.0 / screenHeight * 1.0) / (frameHeight * 1.0 / screenWidth * 1.0);
	textures[4] = 0.0f;
	textures[5] = 0.0f;
	textures[6] = 1.0f;
	textures[7] = 0.0f;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth, frameHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

int ConcreteApp::Process()
{
	if (!capture.isOpened()) {
		return 0;
	}

	capture.read(inframe);
	if (!inframe.empty()) {
		DrawBackground();
	}

	return 1;
}

void ConcreteApp::OnFocusGained()
{
	LOGD("OnFocusGained()");
}

void ConcreteApp::OnFocusLost()
{
	LOGD("OnFocusLost()");
}

void ConcreteApp::DrawBackground() {
	int frameWidth = GetFrameWidth();
	int frameHeight = GetFrameHeight();

	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, texture);

	cvtColor(inframe, outframe, CV_BGR2BGR565);
	cv::flip(outframe, rgbFrame, 1);

	if (texture != 0) {
		LOGD("glTexSubImage2D");
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, rgbFrame.ptr());
	}

	LOGD("glEnableClientState");
	glEnableClientState(GL_VERTEX_ARRAY);
	LOGD("glEnableClientState");
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	LOGD("glLoadIdentity");
	glLoadIdentity();
	// Set the face rotation
	LOGD("glFrontFace");
	glFrontFace(GL_CW);
	// Point to our vertex buffer
	LOGD("glVertexPointer");
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	LOGD("glTexCoordPointer");
	glTexCoordPointer(2, GL_FLOAT, 0, textures);
	// Draw the vertices as triangle strip
	LOGD("glDrawArrays");
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	// Disable the client state before leaving
	LOGD("glDisableClientState");
	glDisableClientState(GL_VERTEX_ARRAY);
	LOGD("glDisableClientState");
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	eglSwapBuffers(display, surface);
}
