package ch.bfh.cpvr.ardoor;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import ch.bfh.cpvr.ardoor.opengl.ARDoorView;

public class MainActivity extends Activity {
	
    private static final String TAG = "ARDoor::MainActivity";
    
    private ARDoorView arDoorView;

    public MainActivity() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        arDoorView = new ARDoorView(this);
        setContentView(arDoorView);
    }

}
