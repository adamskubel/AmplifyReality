package com.amplifyreality;

import android.app.NativeActivity;
import android.content.Context;
import android.content.res.Configuration;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.location.LocationManager;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;

import com.amplifyreality.networking.ARClient;

public class AmplifyRealityActivity extends NativeActivity //implements AutoFocusCallback
{
	static
	{
		System.loadLibrary("amplify_reality");
		Log.i("AmplifyR", "Library loaded.");
	}

	public static native void OnMessage(String messageString, Object data);

	public static native String GetConnectionString();
	
	public static native void SetClientObject(Object amplifyRealityActivity, Object arClient);

	private ARClient client;
//	volatile boolean waiting = true;
	private LocationCollector locationCollector;
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{

		locationCollector = new LocationCollector();
		LocationManager locationManager = (LocationManager) this.getSystemService(Context.LOCATION_SERVICE);
		locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 10, 0,locationCollector);
		locationCollector.onLocationChanged(locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER));

		Log.i("AmplifyR-JNI", "Creating client");
		client = new ARClient(locationCollector);
		SetClientObject(this, client);
		super.onCreate(savedInstanceState);
	}

//	@Override
//	public void onAutoFocus(boolean success, Camera camera)
//	{
//		Log.i("AmplifyR-JNI", "JNI just " + ((success) ? "FUCKED UP" : "focused the camera") + "LOL");
//		waiting = false;
//	}

	public String getPreference(String key)
	{
		return getSharedPreferences("amplify.reality", MODE_WORLD_READABLE).getString(key, null);
	}

	public void setPreference(String key, String value)
	{
		getSharedPreferences("amplify.reality", MODE_WORLD_READABLE).edit().putString(key, value).commit();
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event)
	{
		return super.dispatchKeyEvent(event);
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		Log.i("AmplifyR-JNI", "Configuration Changed Event: KB=" + newConfig.keyboardHidden);
		super.onConfigurationChanged(newConfig);
	}

	@Override
	protected void onDestroy()
	{
		
		client.Shutdown();
		super.onDestroy();
		System.exit(0);
	}

}



// Camera camera = Camera.open();
// Parameters params = camera.getParameters();
// params.setPreviewSize(800, 480);
// params.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
// camera.setParameters(params);
// camera.startPreview();
// Log.d("AmplifyR-JNI","Starting camera focus");
// camera.autoFocus(this);
// while (waiting)
// {
// try
// {
// Log.d("AmplifyR-JNI","Waiting for focus");
// Thread.sleep(100);
// } catch (InterruptedException e)
// {
// Log.w("AmplifyR-JNI","Thread error!",e);
// break;
// }
// }
// camera.stopPreview();
// camera.release();