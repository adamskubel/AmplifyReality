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
import com.amplifyreality.networking.model.ARObject;
import com.amplifyreality.networking.model.ClientRequest;
import com.amplifyreality.networking.model.DataHeader;
import com.amplifyreality.networking.model.Realm;
import com.amplifyreality.networking.model.RealmPositionWatcher;
import com.amplifyreality.networking.model.WavefrontObj;
import com.amplifyreality.util.Logging;

public class ClientThread implements MessageListener, RealmPositionWatcher
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
	private RealmManager realmManager;

	private enum ClientStates
	{
		WaitingForCode, Active;
	}

	private Realm currentRealm;

	private ClientStates clientState = ClientStates.WaitingForCode;

	public ClientThread(Socket _clientSocket, RealmManager realmManager)
	{
		messageInputQueue = new LinkedBlockingQueue<String>();
		messageOutputQueue = new LinkedBlockingQueue<ClientMessage>();

		this.realmManager = realmManager;
		listening = true;
		clientSocket = _clientSocket;

		LOGGER.info("Created new client. Remote address = " + clientSocket.getRemoteSocketAddress().toString());

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
								dataHeader = null;
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
						LOGGER.severe("RemoteClientSenderThread: Interrupted while taking message from queue - " + e.getMessage());
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

	private void ProcessARUpdate(ARObject arObject)
	{
		if (clientState == ClientStates.Active)
		{
			if (arObject.Action != null && arObject.Action.equals("Update"))
			{
				currentRealm.UpdateObject(arObject.Name, arObject);
				LOGGER.info("Updating ARObject with name=" + arObject.Name + ", newposition=" + arObject.Position.toString());
			} else if (arObject.Action.equals("Create"))
			{
				currentRealm.AddNewObject(arObject.Name, arObject);
				LOGGER.info("Creating ARObject with name=" + arObject.Name + ", modelname=" + arObject.ModelName);
			}
		}
	}

	private void ProcessData(DataHeader dataHeader, BufferedReader bufferedReader) throws IOException
	{
		char buffer[] = new char[dataHeader.NumBytes];
		int result = bufferedReader.read(buffer, 0, dataHeader.NumBytes);
		// LOGGER.info("Received " + result + " bytes from client.");
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

				if (className.equals(ClientRequest.class.getCanonicalName()))
				{
					ClientRequest request = serializer.read(ClientRequest.class, data);
					ProcessClientRequest(request);
				} else if (className.equals(ARObject.class.getCanonicalName()))
				{
					ARObject arObject = serializer.read(ARObject.class, data);
					ProcessARUpdate(arObject);
				}
			} catch (Exception e)
			{

				LOGGER.log(Level.SEVERE, "Error parsing message from server. Data=" + data, e);
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
				Realm requestedRealm = realmManager.RequestRealm(request.RequestData);
				// requestedRealm.AddClient(this);
				LOGGER.info("Sending realm to client, RealmData:" + requestedRealm.Name + ". Client is now active.");
				clientState = ClientStates.Active;
				SendMessage(new ClientXMLMessage(new Persister(), requestedRealm));
				currentRealm = requestedRealm;
				currentRealm.AddClient(this);
			} else if (request.RequestType.equals("Resource"))
			{
				if (clientState == ClientStates.Active)
				{
					String objData = realmManager.GetObjFile(request.RequestData);
					LOGGER.info("Sending resource with name " + request.RequestData);
					SendMessage(new ClientXMLMessage(new Persister(), new WavefrontObj(request.RequestData, objData)));
				} else
				{
					LOGGER.log(Level.WARNING, "Inactive client tried to access resource! Remote address is " + clientSocket.getRemoteSocketAddress());
				}
			} else if (request.RequestType.equals("Authenticate"))
			{
				String[] splitData = request.RequestData.split(":");
				if (splitData.length < 2)
				{
					LOGGER.log(Level.INFO, "Failed to authenticate due to invalid auth message. Data=" + request.RequestData);
					SendMessage(new com.amplifyreality.networking.message.ClientStringMessage("AuthFail"));
				}
				String user = splitData[0];
				String pass = splitData[1];

				if (!realmManager.authenticateUser(user, pass))
				{
					LOGGER.log(Level.INFO, "Failed to authenticate user = " + user);
					SendMessage(new com.amplifyreality.networking.message.ClientStringMessage("AuthFail"));
				} else
				{
					LOGGER.log(Level.INFO, "Authenticated user = " + user);
					SendMessage(new com.amplifyreality.networking.message.ClientStringMessage("AuthPass"));
				}

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

	@Override
	public void UpdateObject(String objectId, ARObject newObjectProperties)
	{
		newObjectProperties.Name = objectId;
		try
		{
			SendMessage(new ClientXMLMessage(new Persister(), newObjectProperties));
		} catch (IOException e)
		{
			e.printStackTrace();
			LOGGER.severe(e.getMessage());
		}
	}

}
