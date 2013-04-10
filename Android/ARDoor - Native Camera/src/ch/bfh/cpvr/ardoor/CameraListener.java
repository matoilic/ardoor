package ch.bfh.cpvr.ardoor;

import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.Mat;

public class CameraListener implements CvCameraViewListener2 {
	
	private ARDoorLib arDoorLib = new ARDoorLib();
	private Mat mInput;
	private Mat mOutput;
	
	private boolean passThrough = false;

	@Override
	public void onCameraViewStarted(int width, int height) {}

	@Override
	public void onCameraViewStopped() {}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		if (passThrough) {
			return inputFrame.rgba();
		}
		
		mInput = inputFrame.gray();
		mOutput = arDoorLib.processCanny(mInput);
		return mOutput;
	}
	
	public boolean isPassThrough() {
		return passThrough;
	}

	public void setPassThrough(boolean passThrough) {
		this.passThrough = passThrough;
	}
}
