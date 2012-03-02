package com.amplifyreality.networking;

import java.net.*;
import java.io.*;
import java.util.*;
import java.util.logging.*;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import com.amplifyreality.util.Logging;

public class ARServer 
{	
	private List<ClientThread> broadcastList;
	private volatile boolean listening;

	private RealmManager realmManager;
	String filename = "AugmentedRealms.xml";

	private final static Logger LOGGER = Logging.CreateLogger(ARServer.class);
	
	public ARServer()
	{
		LOGGER.log(Level.INFO,"Starting server");		
		
		broadcastList = new ArrayList<ClientThread>();
		
		listening = true;
		
		Serializer serializer = new Persister();	
		
		
		try
		{
			realmManager = serializer.read(RealmManager.class, new File(filename));
			LOGGER.info("Loaded " + realmManager.NumRealms() + " realms from file.");
		}
		catch (Exception e)
		{
			e.printStackTrace();
			System.out.println("Error reading realm file. Creating new manager.");
			realmManager = new RealmManager();
		}
		
		
		Thread persistenceThread = new Thread(new Runnable()
		{
			public void run()
			{
				while(true)
				{
					if (realmManager.NeedsPersist())
					{
						Serializer serializer = new Persister();
						try
						{
							serializer.write(realmManager, new File(filename));
						} catch (Exception e)
						{
							e.printStackTrace();
						}
					}
					try
					{
						Thread.sleep(5000);
					} catch (InterruptedException e)
					{
						e.printStackTrace();
					}
				}
			}
		});
		
		persistenceThread.start();
		
		
		
		//Thread to listen for connecting clients
		Thread serverThread = new Thread(new Runnable()
		{
			public void run()
			{
				try
				{
					LOGGER.log(Level.INFO,"Waiting for client connection");
					ServerSocket ss = new ServerSocket(12312);
					while (listening)
					{
						// Wait for connection attempt, then spawn a new thread
						Socket clientSocket = ss.accept();
						broadcastList.add(new ClientThread(clientSocket, realmManager));						
					}
					ss.close();
					LOGGER.log(Level.INFO,"Server socket closed");

				}  catch (IOException e)
				{
					LOGGER.log(Level.SEVERE,"Error while waiting for client",e);
				} 
			}

		});
		serverThread.start();
	}
		

	
	public static void main(String[] args)
	{
		new ARServer();		
	}
}
