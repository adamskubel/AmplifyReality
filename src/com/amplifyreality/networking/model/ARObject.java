package com.amplifyreality.networking.model;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

@Root
public class ARObject
{

	@Attribute
	public String Name;
	
	@Attribute
	public String ModelName;

	public ARObject()
	{
		Position = new Vector3();
	}
	
	public ARObject(String Name, Vector3 Position, Vector3 Rotation, Vector3 Scale)
	{
		this.Name = Name;
		this.Position = Position;
		this.Rotation = Rotation;
		this.Scale = Scale;
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
