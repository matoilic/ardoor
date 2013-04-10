package ch.bfh.cpvr.ardoor;

import android.app.NativeActivity;
import android.util.Log;
import android.view.Menu;

public class NativeActivityBridge extends NativeActivity {

	static {
		System.loadLibrary("opencv_java");
	}
	
    private static final String TAG = "ARDoor::NativeActivityBridge";

	
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Log.i(TAG, "called onCreateOptionsMenu");
        menu.add("Test Menu in Native Activity");
        return true;
    }
}
