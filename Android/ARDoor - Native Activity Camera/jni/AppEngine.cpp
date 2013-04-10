#include <stdlib.h>

#include "AppEngine.h"
#include "Logging.h"

void AppEngine::Main()
{
	InitApp();
	MainLoop();
}

int AppEngine::GetFrameWidth()
{
	return frameWidth;
}

int AppEngine::GetFrameHeight()
{
	return frameHeight;
}

void AppEngine::InitApp()
{
	LOGD("Initialize app...");

	app->userData = this;
	app->onAppCmd = AppEngine::HandleCommand;
	OnInitialize();
}

void AppEngine::MainLoop()
{
	LOGD("Beginning main loop...");

	int ret;
    int ident;
    int events;
    struct android_poll_source* source;

	// Main loop
	while (1) {
		// Process events
		while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
			// Process this event.
			if (source != NULL) {
				source->process(this->app, source);
			}

			// Check if we are exiting.
			if (this->app->destroyRequested != 0) {
				TerminateDisplay();
				exit(0);
			}
		}

		if (active) {
			ret = Process();
			if (!ret) {
				LOGD("Process() returned 0, exiting...");
				exit(0);
			}
		}
	}
}

void AppEngine::InitDisplay()
{
	// initialize OpenGL ES and EGL

	/*
	 * Here specify the attributes of the desired configuration.
	 * Below, we select an EGLConfig with at least 8 bits per color
	 * component compatible with on-screen windows
	 */
	const EGLint contextAttribs[] =
	{
//	    EGL_CONTEXT_CLIENT_VERSION, 2,
	    EGL_NONE
	};
	const EGLint configAttribs[] =
	{
	    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
//	    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_BLUE_SIZE, 5,
		EGL_GREEN_SIZE, 6,
		EGL_RED_SIZE, 5,
		EGL_NONE
	};

	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Here, the application chooses the configuration it desires. In this
	 * sample, we have a very simplified selection process, where we pick
	 * the first EGLConfig that matches our criteria */
	eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);

	LOGI("numConfigs: %d", numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	 * As soon as we picked a EGLConfig, we can safely reconfigure the
	 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, app->window, NULL);
	if (surface == NULL)
	    LOGW("error creating window surface: 0x%x", eglGetError());

	context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
	if (context == NULL)
		LOGW("error creating main context: 0x%x", eglGetError());

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	this->display = display;
	this->context = context;
	this->surface = surface;
	this->frameWidth = w;
	this->frameHeight = h;

	// Initialize GL state.
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	//glEnable(GL_CULL_FACE);
	//glShadeModel(GL_SMOOTH);
	//glDisable(GL_DEPTH_TEST);

	OnInitDisplay();
}

void AppEngine::TerminateDisplay() {
	LOGD("Terminate display...");

    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context != EGL_NO_CONTEXT) {
            eglDestroyContext(display, context);
        }
        if (surface != EGL_NO_SURFACE) {
            eglDestroySurface(display, surface);
        }
        eglTerminate(display);
    }
    active = 0;
    display = EGL_NO_DISPLAY;
    surface = EGL_NO_SURFACE;
    context = EGL_NO_CONTEXT;
}

void AppEngine::HandleCommand(AndroidApp* app, int32_t cmd)
{
	AppEngine* engine = (AppEngine*) app->userData;

	switch (cmd) {
		case APP_CMD_SAVE_STATE:
			break;

		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			if (engine->app->window != NULL) {
				engine->InitDisplay();
			}
			break;

		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			engine->TerminateDisplay();
			break;

		case APP_CMD_GAINED_FOCUS:
			engine->active = 1;
			engine->OnFocusGained();
			break;

		case APP_CMD_LOST_FOCUS:
			engine->active = 0;
			engine->OnFocusLost();
			break;
	}
}

void AppEngine::OnInitialize() {}
void AppEngine::OnInitDisplay() {}
void AppEngine::OnFocusGained() {}
void AppEngine::OnFocusLost() {}
