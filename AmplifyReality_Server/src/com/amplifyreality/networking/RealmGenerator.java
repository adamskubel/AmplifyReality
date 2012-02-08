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
		object1.ModelName = "cube.obj";
		object1.Position = new Vector3(50,0,0);
		object1.Scale = new Vector3(35,35,35);
		object1.Rotation = new Vector3(0,0,0);
		
		ARObject object2 = new ARObject();
		object2.Name = "TestObj2";
		object2.ModelName = "cube.obj";
		object2.Position = new Vector3(-50,0,0);
		object2.Scale = new Vector3(40,40,40);
		object2.Rotation = new Vector3(-1.1f,2.1f,3.1f);
		
		
		//newRealm.objectMap.put("Obj1",object1);
		//newRealm.objectMap.put("Obj2",object2);
		
		Serializer serializer = new Persister();
		serializer.write(newRealm, new File(filename));
	}

}
