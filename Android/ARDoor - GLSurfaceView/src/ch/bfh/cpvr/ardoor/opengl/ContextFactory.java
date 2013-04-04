package ch.bfh.cpvr.ardoor.opengl;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

import android.opengl.GLSurfaceView;
import android.util.Log;

class ContextFactory implements GLSurfaceView.EGLContextFactory {

	private static String TAG = "ARDoor::ContextFactory";

    private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    
    public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
        Log.i(TAG, "creating OpenGL ES 2.0 context");
        GLUtil.checkEglError(TAG, "Before eglCreateContext", egl);
        
        int[] attrib_list = {
        		EGL_CONTEXT_CLIENT_VERSION, 2,
        		EGL10.EGL_NONE
        };
        EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
        GLUtil.checkEglError(TAG, "After eglCreateContext", egl);

        return context;
    }

    public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
    	Log.i(TAG, "destroying OpenGL context");
        egl.eglDestroyContext(display, context);
    }
}
