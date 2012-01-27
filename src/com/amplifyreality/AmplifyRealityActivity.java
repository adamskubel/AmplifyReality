package com.amplifyreality;

import android.app.NativeActivity;
import android.os.Bundle;

public class AmplifyRealityActivity extends NativeActivity
{
	static 
	{
        System.loadLibrary("amplify_reality");
	}
	
	public native void SomeFunction();
	
	@Override
    protected void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
		SomeFunction();
		SomeFunction();
	}
	
	
}