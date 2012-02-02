package com.amplifyreality.networking;

import java.io.*;
import java.util.HashMap;
import java.util.logging.FileHandler;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;
import java.util.logging.StreamHandler;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.*;

import com.amplifyreality.networking.model.Realm;

public class RealmManager
{
	////Statics
	private final static Logger LOGGER = Logger.getLogger(RealmManager.class.getName());
	static
	{
		LOGGER.addHandler(new StreamHandler(System.out,new SimpleFormatter()));
		try
		{
			LOGGER.addHandler(new FileHandler("ARServer.log"));
		} catch (Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private static RealmManager instance;
	public static RealmManager getInstance()
	{
		if (instance == null)
			instance = new RealmManager();
		
		return instance;
	}
	///END statics
	

	private HashMap<String,	Realm> realmMap;
	
	public RealmManager()
	{
		realmMap = new HashMap<String,Realm>();
		
	}
	
	public Realm RequestRealm(String code) throws Exception
	{
		if (realmMap.containsKey(code))
			return realmMap.get(code);
		else
			return CreateRealm(code);
	}
	
	private File CodeToFile(String code)
	{
		return new File(code);
	}
	
	private Realm CreateRealm(String code) throws Exception
	{
		File realmFile = CodeToFile(code);
		if (!realmFile.exists())
		{
			LOGGER.warning("Realm with code " + code + " not found.");
			throw new FileNotFoundException();
		}
				
		Serializer xmlDeserializer = new Persister();
		Realm newRealm = xmlDeserializer.read(Realm.class, realmFile);
		LOGGER.info("Created new realm with code: " + code);
		realmMap.put(code, newRealm);
		return newRealm;
	}
	
	
	public byte[] GetObjFile(String code)
	{
		try
		{
			File file = new File("resources/cube.obj");
			LOGGER.info("Sending file data from URI = " + file.getAbsolutePath());
			return java.nio.file.Files.readAllBytes(file.toPath());
			
		} catch (IOException e)
		{
			e.printStackTrace();
		}
		return null;				
	}
	
	
	
	
	
	
	
}
