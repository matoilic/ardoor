#include "ConcreteApp.h"
#include "Logging.h"

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

cv::VideoCapture capture;
cv::Mat rgbFrame;
cv::Mat inframe;
cv::Mat outframe;

GLuint texture;
GLfloat vertices[] = {
	  -1.0f, -1.0f, 0.0f, // V1 - bottom left
	  -1.0f,  1.0f, 0.0f, // V2 - top left
	   1.0f, -1.0f, 0.0f, // V3 - bottom right
	   1.0f,  1.0f, 0.0f  // V4 - top right
};
GLfloat textures[] = {
        // Mapping coordinates for the vertices
        0.0f, 1.0f,     // top left     (V2)
        0.0f, 0.0f,     // bottom left  (V1)
        1.0f, 1.0f,     // top right    (V4)
        1.0f, 0.0f      // bottom right (V3)
};

void checkGLErrors(const char *label) {
    GLenum errCode;
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        LOGE("OpenGL ERROR at %s(): 0x%x", label, errCode);
    }
}


void ConcreteApp::OnInitialize()
{
	LOGD("OnInitialize()");
}

void ConcreteApp::OnInitDisplay()
{
	LOGD("OnInitDisplay()");

	int frameWidth = GetFrameWidth();
	int frameHeight = GetFrameHeight();

	LOGD("Frame size: %dx%d", frameWidth, frameHeight);

	capture.open(CV_CAP_ANDROID + 0);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, frameHeight);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, frameWidth);

	glEnable(GL_TEXTURE_2D);            //Enable Texture Mapping ( NEW )
	glShadeModel(GL_SMOOTH);            //Enable Smooth Shading
	checkGLErrors("glShadeModel");

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);    //Black Background
	checkGLErrors("glClearColor");

	glClearDepthf(1.0f);                     //Depth Buffer Setup
	checkGLErrors("glClearDepthf");

	glEnable(GL_DEPTH_TEST);            //Enables Depth Testing
	checkGLErrors("glEnable");

	glDepthFunc(GL_LEQUAL);             //The Type Of Depth Testing To Do
	checkGLErrors("glDepthFunc");

	// Really Nice Perspective Calculations
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	checkGLErrors("glHint");

	// generate one texture pointer
	glGenTextures(1, &texture);
	checkGLErrors("glGenTextures");

    // ...and bind it
	glBindTexture(GL_TEXTURE_2D, texture);
	checkGLErrors("glBindTexture");

	// create nearest filtered texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	checkGLErrors("glTexParameterf");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	checkGLErrors("glTexParameterf");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
	checkGLErrors("glTexImage2D");
	glBindTexture(GL_TEXTURE_2D, 0);
	checkGLErrors("glBindTexture");

	glViewport(0, 0, frameWidth, frameHeight);
	checkGLErrors("glViewport");

	glMatrixMode(GL_PROJECTION);
	checkGLErrors("glMatrixMode");

	glLoadIdentity();
	checkGLErrors("glLoadIdentity");

	float aspect = frameWidth / frameHeight;
	float bt = (float) tan(45 / 2);
	float lr = bt * aspect;

	//glFrustumf(-lr * 0.1f, lr * 0.1f, -bt * 0.1f, bt * 0.1f, 0.1f, 100.0f);
	glFrustumf(-1.0f, 1.0f, -1.0f, 1.0f, 0.5f, 100.0f);
	checkGLErrors("glFrustumf");

	glMatrixMode(GL_MODELVIEW);
	checkGLErrors("glMatrixMode");

	glLoadIdentity();
	checkGLErrors("glLoadIdentity");

}

int ConcreteApp::Process()
{
	if (!capture.isOpened()) {
		return 0;
	}

	//capture.read(inframe);
	if (!capture.grab()) {
		capture.retrieve(inframe, CV_CAP_ANDROID_COLOR_FRAME_RGBA);
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

	cvtColor(inframe, outframe, CV_RGBA2BGR565);
	//cvtColor(inframe, outframe, CV_BGR2RGB);

	cv::Size dstSize = cv::Size(1024, 1024);
	cv::resize(outframe, rgbFrame, dstSize, 0, 0, cv::INTER_LINEAR);

	//cv::imwrite("/mnt/sdcard/inframe.jpg", inframe);
	//cv::imwrite("/mnt/sdcard/outframe.jpg", outframe);
	//cv::imwrite("/mnt/sdcard/rgbFrame.jpg", rgbFrame);

	//LOGD("write: %d", write);
	//exit(0);

	// clear Screen and Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	checkGLErrors("glClear");

	// Reset the Modelview Matrix
	glLoadIdentity();
	checkGLErrors("glLoadIdentity");

	// Drawing
	glRotatef(-90, 0, 0, 1);
	glTranslatef(0.0f, 0.0f, -0.5f);     // move 5 units INTO the screen
	                                     // is the same as moving the camera 5 units away

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, texture);
	checkGLErrors("glBindTexture");

	if (texture != 0) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 1024, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, rgbFrame.ptr());
		checkGLErrors("glTexSubImage2D");
	}

	// Point to our buffers
	glEnableClientState(GL_VERTEX_ARRAY);
	checkGLErrors("glEnableClientState");

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	checkGLErrors("glEnableClientState");

	// Set the face rotation
	glFrontFace(GL_CW);

	// Point to our vertex buffer
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	checkGLErrors("glVertexPointer");

	glTexCoordPointer(2, GL_FLOAT, 0, textures);
	checkGLErrors("glTexCoordPointer");

	// Draw the vertices as triangle strip
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	checkGLErrors("glDrawArrays");

	// Disable the client state before leaving
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	checkGLErrors("glDisableClientState");

	glDisableClientState(GL_VERTEX_ARRAY);
	checkGLErrors("glDisableClientState");

	eglSwapBuffers(display, surface);
	checkGLErrors("eglSwapBuffers");
}
