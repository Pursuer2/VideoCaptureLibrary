package project.xplat.test;

import android.app.Activity;
import android.widget.TextView;

public class VideoCaptureActivity extends Activity{
	@Override
	protected void onStart() {
		super.onStart();
		TextView tv = new TextView(this);
		setContentView(tv);
		tv.setText("data writing into /sdcard/test.log");
		nativeDemoEntry();
	}
	public native static void nativeDemoEntry();
}
