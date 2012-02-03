package com.amplifyreality.networking.model;

import java.util.*;

import org.simpleframework.xml.*;

import android.util.Log;

import com.amplifyreality.networking.model.*;

//Represents a virtual, augmented world
@Root
public class Realm 
{

	@Attribute
	public String Name;

	@ElementMap
	Map<String, ARObject> objectMap;


	public Realm()
	{
		objectMap = new HashMap<String, ARObject>();
	}
	
	@Override
	public String toString()
	{
		return Name + "=" + String.valueOf(objectMap);
	}
	
//	public ARObject[] getObjectArray()
//	{
//		return new ArrayList<ARObject>(objectMap.values()).toArray(new ARObject[1]);
//	}
	
	public ArrayList getObjectList()
	{
		Log.i("AmplifyR-JNI","Returning ARObjects as array");
		ArrayList list = new ArrayList();
		for (ARObject value : objectMap.values())
		{
			list.add(value);
		}
		return list;
	}
}
