package com.amplifyreality;

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
	public static native Object[] GetOutgoingMessages();
	public static native String GetConnectionString();
	public static native void SetConnected(boolean connected);
	
	public static native void SetKeyboardState(boolean isOpen);
	
//	public static native void SetJavaEnv(ARClient client);
	
	ARClient client;
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		Log.i("AmplifyR-JNI", "Creating client");
		client = new ARClient();

//		InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
//		java.util.List<InputMethodInfo> inputList = imm.getEnabledInputMethodList();
//
//		for (InputMethodInfo inputMethod : inputList)
//		{
//			Log.i("AmplifyR-JNI", "Inputmethod:" + inputMethod.toString());
//		}
		
	}
	
	@Override 
	public boolean dispatchKeyEvent(KeyEvent event)
	{
//		Log.i("AmplifyR-JNI","Configuration Changed Event: KB=" + getSystemService().keyboardHidden);
//
//	    if (newConfig.keyboardHidden == Configuration.KEYBOARDHIDDEN_NO)
//	    {
//	    	SetKeyboardState(true);
//	    }
//	    else
//	    {
//	    	SetKeyboardState(false);
//	    }
//	    
		return super.dispatchKeyEvent(event);
	}
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) 
	{
		Log.i("AmplifyR-JNI","Configuration Changed Event: KB=" + newConfig.keyboardHidden);

	    if (newConfig.keyboardHidden == Configuration.KEYBOARDHIDDEN_NO)
	    {
	    	SetKeyboardState(true);
	    }
	    else
	    {
	    	SetKeyboardState(false);
	    }
	    
//	    // Checks whether a hardware keyboard is available
//	    if (newConfig.hardKeyboardHidden == Configuration.HARDKEYBOARDHIDDEN_NO) {
//	        Toast.makeText(this, "keyboard visible", Toast.LENGTH_SHORT).show();
//	    } else if (newConfig.hardKeyboardHidden == Configuration.HARDKEYBOARDHIDDEN_YES) {
//	        Toast.makeText(this, "keyboard hidden", Toast.LENGTH_SHORT).show();
//	    }

	    super.onConfigurationChanged(newConfig);
	}
	

	@Override 
	protected void onDestroy()
	{
		client.Shutdown();
		super.onDestroy();
	}
	
	
}