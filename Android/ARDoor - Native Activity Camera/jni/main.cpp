#include <jni.h>
#include "ConcreteApp.h"

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
extern "C" {
	void android_main(AndroidApp* state) {
		// Make sure glue isn't stripped.
		app_dummy();

		ConcreteApp app(state);
		app.Main();
	}
}
