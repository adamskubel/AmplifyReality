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
		String filename = "AugmentedRealms.xml";

		Serializer serializer = new Persister();
		
		
		RealmManager manager;
		try
		{
			manager = serializer.read(RealmManager.class, new File(filename));
		} catch (Exception e)
		{
			e.printStackTrace();
			System.out.println("Error reading realm file. Creating new manager.");
			manager = new RealmManager();
		}
				
		manager.AddUser("AdamSkubel", "myPassword");
		manager.AddUser("TestUser", "pass1");
		manager.AddUser("TestUser2", "pass2");

		manager.AddRealm(CreateRealm1());
		manager.AddRealm(CreateRealm2());		

		serializer.write(manager, new File(filename));	
		System.out.println("Realms saved. Exiting");
	}

	
	private static Realm CreateTestRealm() throws Exception
	{
		Realm newRealm = new Realm("TestRealm_Name",3.0f);
		
		ARObject object1 = new ARObject();
		object1.Name = "TestObj1";
		object1.ModelName = "cube.obj";
		object1.Position = new Vector3(40,0,0);
		object1.Scale = new Vector3(15,10,15);
		object1.Rotation = new Vector3(0,0,0);
		object1.BoundingSphereRadius = 15;
		
		ARObject object2 = new ARObject();
		object2.Name = "TestObj2";
		object2.ModelName = "cube.obj";
		object2.Position = new Vector3(-40,0,0);
		object2.Scale = new Vector3(15,20,10);
		object2.Rotation = new Vector3(-1.1f,2.1f,3.1f);
		object2.BoundingSphereRadius = 18;
		
		
		newRealm.objectMap.put(object1.Name,object1);
		newRealm.objectMap.put(object2.Name,object2);
		
		return newRealm;
		
	}
	
	private static Realm CreateRealm1() throws Exception
	{

		Realm newRealm = new Realm("ThisIsAQRCode",3.0f);
		
		ARObject object1 = new ARObject();
		object1.Name = "TestObj1";
		object1.ModelName = "cube.obj";
		object1.Position = new Vector3(40,0,0);
		object1.Scale = new Vector3(15,10,15);
		object1.Rotation = new Vector3(0,0,0);
		object1.BoundingSphereRadius = 15;
		
		ARObject object2 = new ARObject();
		object2.Name = "TestObj2";
		object2.ModelName = "cube.obj";
		object2.Position = new Vector3(-40,0,0);
		object2.Scale = new Vector3(15,20,10);
		object2.Rotation = new Vector3(-1.1f,2.1f,3.1f);
		object2.BoundingSphereRadius = 18;
		
		
		newRealm.objectMap.put(object1.Name,object1);
		newRealm.objectMap.put(object2.Name,object2);
		
		return newRealm;
	}
	
	private static Realm CreateRealm2() throws Exception
	{
		Realm newRealm =  new Realm("TestRealm_2",2.0f);
		ARObject object1 = new ARObject();
		object1.Name = "TestObj1";
		object1.ModelName = "cube.obj";
		object1.Position = new Vector3(20,20,0);
		object1.Scale = new Vector3(15,15,15);
		object1.Rotation = new Vector3(0,0,0);
		object1.BoundingSphereRadius = 20;
		
		ARObject object2 = new ARObject();
		object2.Name = "TestObj2";
		object2.ModelName = "cube.obj";
		object2.Position = new Vector3(-20,20,0);
		object2.Scale = new Vector3(15,15,15);
		object2.Rotation = new Vector3(0,0,0);
		object2.BoundingSphereRadius = 20;
		

		ARObject object3 = new ARObject();
		object3.Name = "TestObj3";
		object3.ModelName = "cube.obj";
		object3.Position = new Vector3(0,-20,0);
		object3.Scale = new Vector3(15,15,15);
		object3.Rotation = new Vector3(0,0,0);
		object3.BoundingSphereRadius = 20;

		newRealm.objectMap.put(object1.Name,object1);
		newRealm.objectMap.put(object2.Name,object2);
		newRealm.objectMap.put(object3.Name,object3);
		

		return newRealm;
	}
}
