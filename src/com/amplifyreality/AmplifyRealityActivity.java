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
	}
	
	public static native void OnMessage(String messageString, Object data);
	public static native Object[] GetOutgoingMessages();
	
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