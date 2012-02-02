package com.amplifyreality.networking;

import java.io.*;
import java.util.HashMap;
import java.util.logging.FileHandler;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;
import java.util.logging.StreamHandler;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.*;

import com.amplifyreality.networking.model.*;

public class RealmGenerator
{

	/**
	 * @param args
	 * @throws Exception 
	 */
	public static void main(String[] args) throws Exception
	{
		String filename = "TestRealm.xml";
		Realm newRealm = new Realm();
		
		newRealm.Name = "TestRealm_Name";
		
		ARObject object1 = new ARObject();
		object1.Name = "TestObj1";
		object1.Position = new Vector3(1,1,1);
		object1.Scale = new Vector3(-2,-2,-2);
		object1.Rotation = new Vector3(1.0f,2.0f,3.0f);
		
		ARObject object2 = new ARObject();
		object2.Name = "TestObj2";
		object2.Position = new Vector3(31,31,11);
		object2.Scale = new Vector3(-21,-21,-21);
		object2.Rotation = new Vector3(-1.1f,2.1f,3.1f);
		
		
		newRealm.objectMap.put("Obj1",object1);
		newRealm.objectMap.put("Obj2",object2);
		
		Serializer serializer = new Persister();
		serializer.write(newRealm, new File(filename));
	}

}
