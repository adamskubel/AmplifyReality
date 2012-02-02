package com.amplifyreality.networking.model;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Root;

@Root
public class Vector3
{
	@Attribute
	public float X;
	@Attribute
	public float Y;
	@Attribute
	public float Z;
	
	
	public Vector3()
	{
		X = Y = Z = 0;
	}
	
	public Vector3(float X, float Y, float Z)
	{
		this.X = X;
		this.Y = Y;
		this.Z = Z;
	}
	
	
	@Override 
	public String toString()
	{
		return "[" + X + "," + Y + "," + Z + "]";
	}
}

