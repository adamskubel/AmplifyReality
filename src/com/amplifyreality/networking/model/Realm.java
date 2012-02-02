package com.amplifyreality.networking.model;

import java.util.*;

import org.simpleframework.xml.*;

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
}
