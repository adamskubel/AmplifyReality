package com.amplifyreality.networking;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.logging.Logger;

import org.simpleframework.xml.ElementMap;
import org.simpleframework.xml.Root;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import sun.misc.BASE64Decoder;
import sun.misc.BASE64Encoder;

import com.amplifyreality.networking.exceptions.AuthenticationFailedException;
import com.amplifyreality.networking.exceptions.RealmNotFoundException;
import com.amplifyreality.networking.model.Realm;
import com.amplifyreality.util.Logging;

@Root
public class RealmManager
{
	private final static Logger LOGGER = Logging.CreateLogger(RealmManager.class);

	public boolean needsPersist = false;
	
	
	@ElementMap
	private HashMap<String, Realm> realmMap;

	@ElementMap
	private HashMap<String, String> credentialMap;

	public void AddUser(String user, String password)
	{
		java.security.MessageDigest digest;
		try
		{
			digest = java.security.MessageDigest.getInstance("SHA-256");
		} catch (NoSuchAlgorithmException e)
		{
			e.printStackTrace();
			return;
		}
		byte[] bytes = digest.digest(password.getBytes());
		credentialMap.put(user,new BASE64Encoder().encode(bytes));
	}
	
	public boolean authenticateUser(String user, String plainTextPassword)
	{
		if (!credentialMap.containsKey(user))
			return false;
		if (plainTextPassword == null || plainTextPassword.length() == 0)
			return false;
		

		java.security.MessageDigest digest;
		try
		{
			digest = java.security.MessageDigest.getInstance("SHA-256");
		} catch (NoSuchAlgorithmException e)
		{
			e.printStackTrace();
			return false;
		}
		
		byte[] bytes = digest.digest(plainTextPassword.getBytes());
		String base64encrypted = new BASE64Encoder().encode(bytes);
		
		if (base64encrypted.equals(credentialMap.get(user)))
			return true;
		else
			return false;
	}

	public RealmManager()
	{
		realmMap = new HashMap<String, Realm>();
		credentialMap = new HashMap<String,String>();
	}

	public Realm RequestRealm(String code) throws Exception
	{		
		if (realmMap.containsKey(code))
		{
			return realmMap.get(code);
		}
		else
		{
			Realm newRealm = CreateRealm(code);
			realmMap.put(code,newRealm);
			return newRealm;
		}
	}

	public boolean NeedsPersist()
	{			
		if (realmMap.size() > 0)
		{
			boolean needsUpdate = needsPersist;
			for (Realm updateRealm : realmMap.values())
			{
				needsUpdate = needsUpdate || updateRealm.isUpdated;
				updateRealm.isUpdated = false;
			}
			return needsUpdate;
		} else
		{
			return needsPersist;
		}
	}

	public int NumRealms()
	{
		return realmMap.size();
	}

	public void AddRealm(Realm realm)
	{
		realmMap.put(realm.Name, realm);
	}

	public Realm GetRealm(String code) throws RealmNotFoundException
	{
		if (realmMap.containsKey(code))
			return realmMap.get(code);
		else
			throw new RealmNotFoundException();
	}

	public Realm CreateRealm(String code)
	{
		Realm newRealm = new Realm(code, 10.0f);
		newRealm.isUpdated = true;
		needsPersist = true;
		LOGGER.info("Creating new realm, code = " + code);
		return newRealm;
	}

	//
	// private Realm ReadRealm(String code) throws Exception
	// {
	// File realmFile = CodeToFile(code);
	// if (!realmFile.exists())
	// {
	// LOGGER.warning("Realm with code " + code + " not found. Using default realm.");
	// realmFile = CodeToFile("TestRealm.xml");
	// // throw new FileNotFoundException();
	// }
	//
	// Serializer xmlDeserializer = new Persister();
	// Realm newRealm = xmlDeserializer.read(Realm.class, realmFile);
	// LOGGER.info("Created new realm with code: " + code);
	// realmMap.put(code, newRealm);
	// return newRealm;
	// }

	public String GetObjFile(String code)
	{
		try
		{
			String path = new String("resources/" + code);
			// LOGGER.info("Sending file data, path=" + path);

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
