package ch.bfh.cpvr.ardoor;

import org.opencv.android.CameraBridgeViewBase;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.WindowManager;

public class MainActivity extends Activity {
	
    private static final String TAG = "ARDoor::MainActivity";

    private CameraBridgeViewBase mOpenCvCameraView;
    private CameraListener cameraListener = new CameraListener();

    public MainActivity() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main);

        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.camera_surface_view);

        mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);
        mOpenCvCameraView.setCvCameraViewListener(cameraListener);
        mOpenCvCameraView.enableFpsMeter();
        mOpenCvCameraView.enableView();
    }

    @Override
    public void onPause()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }
    
    @Override
    public void onResume()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.enableView();
    }

    public void onDestroy() {
        super.onDestroy();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		cameraListener.setPassThrough(!cameraListener.isPassThrough());
		return super.onTouchEvent(event);
	}

}
