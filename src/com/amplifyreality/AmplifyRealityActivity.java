package com.amplifyreality;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import com.amplifyreality.networking.ARClient;

import android.app.NativeActivity;
import android.content.Context;
import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.inputmethod.InputMethodInfo;
import android.view.inputmethod.InputMethodManager;

public class AmplifyRealityActivity extends NativeActivity
{	
	static 
	{
        System.loadLibrary("amplify_reality");
       	Log.i("AmplifyR","Library loaded.");
	}
	
	public static native void OnMessage(String messageString, Object data);
	public static native String GetConnectionString();
	public static native void SetClientObject(Object amplifyRealityActivity, Object arClient);
		
	ARClient client;
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		Log.i("AmplifyR-JNI", "Creating client");
		client = new ARClient();
		SetClientObject(this, client);		
		super.onCreate(savedInstanceState);
	}
	
	
	public boolean acceptingText()
	{
		
		try
		{
			InputMethodManager  m = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
			m.showSoftInput(super.getCurrentFocus(), 0);
		}
		catch (Exception e)
		{
			Log.i("AmplifyR-JNI","Exception :" + e.getMessage(),e);
		}
//		try
//		{
//			Method showImeMethod = NativeActivity.class.getMethod("showIme", int.class);		
//			showImeMethod.setAccessible(true);
//			showImeMethod.invoke(this, 0);
//			super.getSystemService(INPUT_METHOD_SERVICE);
//			
//		} catch (SecurityException e)
//		{
//			Log.v("AmplifyR-JNI", "SecurityException",e);
//		} catch (NoSuchMethodException e)
//		{
//
//			Log.v("AmplifyR-JNI","NoSuchMethod",e);
//		} catch (IllegalArgumentException e)
//		{
//			Log.v("AmplifyR-JNI","Reflect",e);
//		} catch (IllegalAccessException e)
//		{
//			Log.v("AmplifyR-JNI","Reflect",e);
//		} catch (InvocationTargetException e)
//		{
//			Log.v("AmplifyR-JNI","Reflect",e);
//		}		

		return true;
	}
	
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
		Log.i("AmplifyR-JNI","Configuration Changed Event: KB=" + newConfig.keyboardHidden);
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