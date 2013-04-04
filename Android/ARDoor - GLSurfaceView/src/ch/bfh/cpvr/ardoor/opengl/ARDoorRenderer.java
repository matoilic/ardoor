package ch.bfh.cpvr.ardoor.opengl;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import ch.bfh.cpvr.ardoor.ARDoorLib;

import android.opengl.GLSurfaceView;
import android.util.Log;

public class ARDoorRenderer implements GLSurfaceView.Renderer {

	private static String TAG = "ARDoor::Renderer";

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Log.i(TAG, "onSurfaceCreated");
        ARDoorLib.onSurfaceCreated();
    }
    
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        Log.i(TAG, "onSurfaceChanged");
        ARDoorLib.onSurfaceChanged(width, height);
    }

    public void onDrawFrame(GL10 gl) {
    	ARDoorLib.onDrawFrame();
	}
}
