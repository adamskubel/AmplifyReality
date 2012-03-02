package com.amplifyreality.networking.model;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

import android.util.Log;

@Root
public class ARObject
{

	@Attribute
	public String Name;
	
	@Attribute(required=false)	
	public String ModelName;

	@Attribute(required=false)
	public String Action;
	
	public ARObject()
	{
		Position = new Vector3();
	}
	
	public ARObject(String Name, Vector3 Position, Vector3 Rotation)
	{
		Log.i("AmplifyR-JNI","Creating update-mode ARObject. Name=" + Name + "Position = " + Position + " Rotation=" + Rotation);
		this.Name = Name;
		this.Position = Position;
		this.Rotation = Rotation;
		Action = "Update";
	}

	
	public ARObject(String Name, Vector3 Position, Vector3 Rotation, Vector3 Scale, float BoundingSphereRadius)
	{
		this.Name = Name;
		this.Position = Position;
		this.Rotation = Rotation;
		this.Scale = Scale;
		this.BoundingSphereRadius = BoundingSphereRadius;
	}

	public void Update(ARObject newObject)
	{
		if (newObject.Position != null)
			Position = newObject.Position;
		if (newObject.Rotation != null)
			Rotation = newObject.Rotation;
	}
	
	@Element
	public Vector3 Position;

	@Element
	public Vector3 Rotation;

	@Element(required=false)
	public Vector3 Scale;

	@Element(required=false)
	public float BoundingSphereRadius;
	
	@Override
	public String toString()
	{
		return Name + ". Position =" + Position.toString();
	}

}
