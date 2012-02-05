package com.amplifyreality.networking;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.logging.Logger;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import com.amplifyreality.networking.model.Realm;
import com.amplifyreality.util.Logging;

public class RealmManager
{
	// //Statics
	private final static Logger LOGGER = Logging.CreateLogger(RealmManager.class);

	private static RealmManager instance;

	public static RealmManager getInstance()
	{
		if (instance == null)
			instance = new RealmManager();

		return instance;
	}

	// /END statics

	private HashMap<String, Realm> realmMap;

	public RealmManager()
	{
		realmMap = new HashMap<String, Realm>();

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
			LOGGER.warning("Realm with code " + code + " not found. Using default realm.");
			realmFile = CodeToFile("TestRealm.xml");
			// throw new FileNotFoundException();
		}

		Serializer xmlDeserializer = new Persister();
		Realm newRealm = xmlDeserializer.read(Realm.class, realmFile);
		LOGGER.info("Created new realm with code: " + code);
		realmMap.put(code, newRealm);
		return newRealm;
	}

	public String GetObjFile(String code)
	{
		try
		{
			String path = new String("resources/" + code);
			LOGGER.info("Sending file data, path=" + path);
			
			byte[] buffer = new byte[(int) new File(path).length()];
			BufferedInputStream reader = new BufferedInputStream(new FileInputStream(path));
			reader.read(buffer);
			reader.close();
			return new String(buffer);

		} catch (IOException e)
		{
			e.printStackTrace();
		}
		return null;
	}

}
