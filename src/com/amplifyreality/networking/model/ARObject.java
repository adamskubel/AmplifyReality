package com.amplifyreality.networking.model;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

@Root
public class ARObject
{

	@Attribute
	public String Name;

	public ARObject()
	{
		Position = new Vector3();
	}

	@Element
	public Vector3 Position;

	@Element
	public Vector3 Rotation;

	@Element
	public Vector3 Scale;

	@Override
	public String toString()
	{
		return Name + ". Position =" + Position.toString();
	}

}
