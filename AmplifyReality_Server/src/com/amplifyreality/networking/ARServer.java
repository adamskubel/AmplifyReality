package com.amplifyreality.networking;

import java.net.*;
import java.io.*;
import java.util.*;
import java.util.logging.*;

public class ARServer 
{	
	private List<ClientThread> broadcastList;
	private volatile boolean listening;

	private final static Logger LOGGER = Logger.getLogger(ARServer.class.getName());
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
	
	public ARServer()
	{
		LOGGER.log(Level.INFO,"Starting server");		
		
		broadcastList = new ArrayList<ClientThread>();
		
		listening = true;
		
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
						broadcastList.add(new ClientThread(clientSocket));						
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
