package com.amplifyreality;

import com.amplifyreality.networking.ARClient;

import android.app.NativeActivity;
import android.os.Bundle;
import android.util.Log;

public class AmplifyRealityActivity extends NativeActivity
{
	
	
	static 
	{
        System.loadLibrary("amplify_reality");
       	Log.i("AmplifyR","Library loaded.");
	}
	
	public static native void OnMessage(String messageString, Object data);
	public static native Object[] GetOutgoingMessages();
	public static native String GetConnectionString();
	public static native void SetConnected(boolean connected);
	
//	public static native void SetJavaEnv(ARClient client);
	
	ARClient client;
	
	@Override
    protected void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
		Log.i("AmplifyR","Creating client");		
		client = new ARClient();
	}
	

	@Override 
	protected void onDestroy()
	{
		client.Shutdown();
		super.onDestroy();
	}
	
	
}