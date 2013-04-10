package ch.bfh.cpvr.ardoor.opengl;

import javax.microedition.khronos.egl.EGL10;
import android.util.Log;

public class GLUtil {
    
    public static void checkEglError(String tag, String prompt, EGL10 egl) {
        int error;
        while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS) {
        	Log.e(tag, String.format("%s: EGL error: 0x%x", prompt, error));
        }
    }
}
