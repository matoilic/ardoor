package ch.bfh.cpvr.ardoor;

import org.opencv.core.Mat;

public class ARDoorLib {
	
	static {
		System.loadLibrary("opencv_java");
		System.loadLibrary("ardoor");
	}
	
    private static native long nativeCanny(long inputImage);
    
    public static native void onSurfaceCreated();
    public static native void onSurfaceChanged(int width, int height);
    public static native void onDrawFrame();

	public Mat processCanny(Mat inputFrame) {
        long matAddress = nativeCanny(inputFrame.getNativeObjAddr());
        
        if (matAddress == 0)
        	return null;
        
        return new Mat(matAddress);
	}

}
