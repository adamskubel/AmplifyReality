package com.amplifyreality.networking;

import java.io.*;
import java.net.URL;
import java.util.logging.FileHandler;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;
import java.util.logging.StreamHandler;

public class RealmManager
{
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
	
	public byte[] GetRealm(String code)
	{
		ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
	//	URL url = classLoader.getResource("cube.obj");
		try
		{
		//	LOGGER.info("Sending file data from URI = " + url.getFile());
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
