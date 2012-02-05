package com.amplifyreality.networking;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import com.amplifyreality.AmplifyRealityActivity;
import com.amplifyreality.networking.exceptions.InvalidHeaderMessageException;
import com.amplifyreality.networking.message.ClientMessage;
import com.amplifyreality.networking.message.ClientXMLMessage;
import com.amplifyreality.networking.model.ClientRequest;
import com.amplifyreality.networking.model.DataHeader;
import com.amplifyreality.networking.model.Realm;
import com.amplifyreality.networking.model.WavefrontObj;
import com.amplifyreality.util.Logging;

public class ClientThread implements MessageListener
{

	volatile boolean listening;
	private Socket clientSocket;

	// Messages from remote client
	// LinkedBlockingQueue<String> remoteMessageQueue;

	// Messages from other clients
	LinkedBlockingQueue<String> messageInputQueue;

	// Messages to this client
	LinkedBlockingQueue<ClientMessage> messageOutputQueue;

	private final static Logger LOGGER = Logging.CreateLogger(ClientThread.class);
	

	private enum ClientStates
	{
		WaitingForCode, Active;
	}

	private ClientStates clientState = ClientStates.WaitingForCode;

	public ClientThread(Socket _clientSocket)
	{
		messageInputQueue = new LinkedBlockingQueue<String>();
		messageOutputQueue = new LinkedBlockingQueue<ClientMessage>();
		// remoteMessageQueue = new LinkedBlockingQueue<String>();

		listening = true;
		clientSocket = _clientSocket;

		LOGGER.info("Created new client. Host=" + clientSocket.getInetAddress().toString());

		// Listen for messages, and process them as they arrive.
		Thread listenThread = new Thread(new Runnable()
		{

			@Override
			public void run()
			{
				DataHeader dataHeader = null;
				try
				{
					BufferedReader input = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
					while (listening)
					{

						if (dataHeader == null)
						{
							String inputLine = input.readLine();
							if (inputLine == null)
							{
								Shutdown();
								break;
							}
							try
							{
								dataHeader = DataHeader.CreateHeaderFromMessage(inputLine);
							} catch (InvalidHeaderMessageException e)
							{
								dataHeader = null; // Ignore line, reset header to null
								ProcessRemoteMessage(inputLine); // Process as a string message
							}
						} else
						{
							ProcessData(dataHeader, input);
							dataHeader = null; // Reset header to null
						}
					}

				} catch (IOException e)
				{
					e.printStackTrace();
					listening = false;
				}

				Cleanup();
			}

		});

		// Process messages arriving from other clients
		Thread messageProcessing = new Thread(new Runnable()
		{

			@Override
			public void run()
			{
				while (listening)
				{
					try
					{
						// Block until a message is available to process
						String message = messageInputQueue.take();
						ProcessMessage(message);

					} catch (InterruptedException e)
					{
						LOGGER.severe("LocalMessageProcessorThread: Interrupted while taking message from queue");
						messageInputQueue.clear();
						Shutdown();
					}
				}
			}

		});

		// Process outgoing messages
		Thread sendMessages = new Thread(new Runnable()
		{

			@Override
			public void run()
			{
				while (listening)
				{
					try
					{
						// Block until a message is available to send
						SendMessage(messageOutputQueue.take());
					} catch (InterruptedException e)
					{
						System.out.println("RemoteClientSenderThread: Interrupted while taking message from queue - " + e.getMessage());
						messageOutputQueue.clear();
						Shutdown();
					} catch (IOException e)
					{
						LOGGER.severe("RemoteClientSenderThread: Error sending message to client - " + e.getMessage());
						messageOutputQueue.clear();
						Shutdown();
					}
				}
			}

		});

		listenThread.start();
		messageProcessing.start();
		sendMessages.start();

	}

	private void ProcessRemoteMessage(String message)
	{
		// String action = message;
		//
		// if (action.equals("SendObjectFile"))
		// {
		// //Resolve file name here
		// byte[] data = RealmManager.getInstance().GetObjFile("filename");
		// if (data != null)
		// {
		// messageOutputQueue.add(new ClientByteMessage(data));
		// }
		// }
		// else if (action.equals("JoinWorld"))
		// {
		// try
		// {
		// Realm realm = RealmManager.getInstance().RequestRealm("TestRealm.xml");
		// Serializer serial = new Persister();
		// messageOutputQueue.add(new ClientXMLMessage(serial,realm));
		// } catch (Exception e)
		// {
		// LOGGER.log(Level.WARNING,"Error retrieving realm.",e);
		// //Notify client of failure
		// }
		// }
	}

	private void ProcessData(DataHeader dataHeader, BufferedReader bufferedReader) throws IOException
	{
		char buffer[] = new char[dataHeader.NumBytes];
		int result = bufferedReader.read(buffer, 0, dataHeader.NumBytes);
		LOGGER.info("Received " + result + " bytes from client.");
		String data = new String(buffer);

		// No XML definition, so process as a string
		if (dataHeader.XmlDataType == null)
		{
			LOGGER.info("Datatype is null, so processing as string: " + data);
			AmplifyRealityActivity.OnMessage(data, null);
		} else
		// Deserialize
		{
			try
			{
				String className = dataHeader.XmlDataType;

				Serializer serializer = new Persister();

				LOGGER.info("Deserializing data, classname=" + className);

				if (className.equals(ClientRequest.class.getCanonicalName()))
				{
					LOGGER.info("Deserializing as client request");
					ClientRequest request = serializer.read(ClientRequest.class, data);
					LOGGER.info("Deserialization complete");
					ProcessClientRequest(request);
				}
			} catch (Exception e)
			{
				
				LOGGER.log(Level.SEVERE, "Error parsing message from server. Data="+ data, e);
			}
		}

	}

	private void ProcessClientRequest(ClientRequest request)
	{
		try
		{
			LOGGER.info("Processing client request. Type=" + request.RequestType);
			if (request.RequestType.equals("Realm"))
			{
				Realm requestedRealm = RealmManager.getInstance().RequestRealm(request.RequestData);
				// requestedRealm.AddClient(this);
				LOGGER.info("Sending realm to client, RealmData:" + requestedRealm.toString());
				SendMessage(new ClientXMLMessage(new Persister(), requestedRealm));
			} else if (request.RequestType.equals("Resource"))
			{
				String objData = RealmManager.getInstance().GetObjFile(request.RequestData);
				LOGGER.info("Sending obj data, datastring(0..20)=" + objData.substring(0, 20));
				SendMessage(new ClientXMLMessage(new Persister(), new WavefrontObj(request.RequestData, objData)));
			}
		} catch (Exception e)
		{
			LOGGER.log(Level.SEVERE, "Error processing client request.", e);
		}
	}

	// Process local messages
	private void ProcessMessage(String message)
	{

	}

	private void SendMessage(ClientMessage message) throws IOException
	{
		if (!IsConnected())
			return;

		message.SendMessage(clientSocket.getOutputStream());

	}

	public void OnMessage(String msg)
	{
		messageInputQueue.add(msg);
	}

	public boolean IsConnected()
	{
		if (clientSocket != null && clientSocket.isConnected())
			return true;
		else
			return false;
	}

	// Synchronous shutdown
	public void Shutdown()
	{
		listening = false;
	}

	private void Cleanup()
	{
		try
		{
			if (clientSocket != null)
				clientSocket.close();
			LOGGER.info("Socket shutdown successfully.");
		} catch (IOException ioe)
		{
			ioe.printStackTrace();
			LOGGER.severe("Error closing socket");
		}
	}

}
