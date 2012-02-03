package com.amplifyreality.networking.model;

public class NativeMessage
{
	
	public String action;
	public Object data;
	
	public NativeMessage(String action, Object data)
	{
		this.action = action;
		this.data = data;
	}
}
