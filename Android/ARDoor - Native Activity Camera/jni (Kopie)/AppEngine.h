#include <android_native_app_glue.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

typedef struct android_app AndroidApp;

class AppEngine
{
private:
	AndroidApp* app;
	int displayInitialized;
	int active;
	int frameWidth;
	int frameHeight;

	void InitApp();
	void MainLoop();
	void InitDisplay();
	void TerminateDisplay();
	static void HandleCommand(AndroidApp* app, int32_t cmd);

protected:
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;

	virtual int Process() = 0;
	virtual void OnInitialize();
	virtual void OnInitDisplay();
	virtual void OnFocusGained();
	virtual void OnFocusLost();

public:
	AppEngine(AndroidApp* app)
	{
		this->app = app;

		this->active = 0;
		this->displayInitialized = 0;
		this->frameWidth = 0;
		this->frameHeight = 0;

		this->context = EGL_NO_CONTEXT;
		this->surface = EGL_NO_SURFACE;
		this->display = EGL_NO_DISPLAY;
	};

	virtual ~AppEngine() {};

	void Main();
	int GetFrameWidth();
	int GetFrameHeight();
};
