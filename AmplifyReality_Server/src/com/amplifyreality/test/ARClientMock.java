package com.amplifyreality.test;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.logging.Logger;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import com.amplifyreality.AmplifyRealityActivity;
import com.amplifyreality.LocationCollector;
import com.amplifyreality.networking.ARServer;
import com.amplifyreality.networking.exceptions.InvalidHeaderMessageException;
import com.amplifyreality.networking.exceptions.UnknownClientActionException;
import com.amplifyreality.networking.message.ClientXMLMessage;
import com.amplifyreality.networking.message.NativeMessage;
import com.amplifyreality.networking.model.ARObject;
import com.amplifyreality.networking.model.ClientRequest;
import com.amplifyreality.networking.model.DataHeader;
import com.amplifyreality.networking.model.MyLocation;
import com.amplifyreality.networking.model.Realm;
import com.amplifyreality.networking.model.WavefrontObj;
import com.amplifyreality.util.Logging;


public class ARClientMock
{


	private final static Logger LOGGER = Logging.CreateLogger(ARClientMock.class);
	
	static String LOGTAG_NETWORKING = "AmplifyR-Networking_Java";

	//Defaults
	String host = "192.168.1.9";
	int port = 12312;
	
	volatile boolean listening = false;

	private LinkedBlockingQueue<Object[]> outgoingQueue;
	private LinkedBlockingQueue<Integer> authenticationResultQueue;
	
	Socket mySocket;
	BufferedReader reader;

	public ARClientMock()
	{
		reader = null;
		outgoingQueue = new LinkedBlockingQueue<Object[]>();
		authenticationResultQueue = new LinkedBlockingQueue<Integer>();
	}

	private void SendNativeMessages(Object[] msgs)
	{

		if (msgs == null)
			return;
		for (int i = 0; i < msgs.length; i++)
		{
			if (msgs[i] == null)
				continue;

			NativeMessage message = (NativeMessage) msgs[i];
			Serializer xmlSerializer = new Persister();

			if (message.isLocationSensitive())
			{
				message.setLocation(null);
			}
			
			try
			{
				ClientXMLMessage msg = new ClientXMLMessage(xmlSerializer, message.GetXMLObject());
				LOGGER.info( "Sending nativemsg: " + message.action);
				try
				{
					msg.SendMessage(mySocket.getOutputStream());
				} catch (IOException e)
				{
					LOGGER.severe( "Error sending message: " + e);
				}
			} catch (UnknownClientActionException e)
			{
				LOGGER.severe("Unknown client action '" + message.action + "'");
			}

		}
	}

	public void QueueNativeMessages(Object[] msgs)
	{
		LOGGER.info("Native layer added " + msgs.length + " messages.");
		outgoingQueue.add(msgs);
	}

	private void RequestData()
	{
		Thread t = new Thread(new Runnable()
		{			
			public void run()
			{
				
				while (listening)
				{
					Object[] msgs;
					try
					{
						msgs = outgoingQueue.take();
					} catch (InterruptedException e)
					{
						LOGGER.severe("Error taking from queue" + e);
						continue;
					}
					SendNativeMessages(msgs);
				}
				
			}			

		});
		t.start();
	}

	private void StartListening()
	{
		LOGGER.info( "Waiting for server messages");
		Thread t = new Thread(new Runnable()
		{
			@Override
			public void run()
			{				
				DataHeader dataHeader = null;
				try
				{
					BufferedReader reader = new BufferedReader(new InputStreamReader(mySocket.getInputStream()));
					while (listening)
					{
						if (dataHeader == null)
						{
							String inputLine = reader.readLine();
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
								
//								Log.d("Inputline=" + inputLine);
								if (authenticationResultQueue != null && inputLine.equals("AuthPass"))
								{
									authenticationResultQueue.add(1);
								}
								else if (authenticationResultQueue != null && inputLine.equals("AuthFail"))
								{
									authenticationResultQueue.add(-1);
								}
								
								AmplifyRealityActivity.OnMessage(inputLine, null); // Process as string
							}
						} else
						{
							ProcessData(dataHeader, reader);
							dataHeader = null;
						}
					}
				} catch (IOException e)
				{
					LOGGER.severe( "IO error: " + e.getMessage());
					Shutdown();
				}
				Cleanup();
			}
		});
		t.start();
	}

	private void ProcessData(DataHeader dataHeader, BufferedReader bufferedReader) throws IOException
	{
		char buffer[] = new char[dataHeader.NumBytes];
		int result = bufferedReader.read(buffer, 0, dataHeader.NumBytes);
		LOGGER.info( "Received " + result + " bytes from server.");
		String data = new String(buffer);

		// No XML definition, so process as a string
		if (dataHeader.XmlDataType == null)
		{			
			AmplifyRealityActivity.OnMessage(data, null);
		} else
		// Deserialize
		{
			try
			{
				String className = dataHeader.XmlDataType;

				Serializer serializer = new Persister();
				if (className.equals(ARObject.class.getCanonicalName())) //Update notification
				{
					ARObject arObject = serializer.read(ARObject.class, data);
					LOGGER.info( "Received ARObject: " + arObject.toString());
					AmplifyRealityActivity.OnMessage("ARObjectUpdate",arObject);					
				} else if (className.equals(Realm.class.getCanonicalName()))
				{
					Realm realm = serializer.read(Realm.class, data);
					LOGGER.info( "Received new Realm: " + realm.toString());
					AmplifyRealityActivity.OnMessage("RealmObject", realm);
				} else if (className.equals(WavefrontObj.class.getCanonicalName()))
				{
					WavefrontObj wavefrontObj = serializer.read(WavefrontObj.class, data);
					LOGGER.info( "Received new wavefront model: " + wavefrontObj.toString());
					AmplifyRealityActivity.OnMessage("WavefrontObject", wavefrontObj);
				} else
				{
					LOGGER.severe( "Received unknown XML object. Classname=" + className);
				}
			} catch (Exception e)
			{
				LOGGER.severe( "Error parsing message from server :" + e);
			}
		}

	}

	public int Connect(String connectionString, String user, String password)
	{
		if (listening)
			return 2;
		
		InetAddress connectHost = null;
		int connectPort = 0;
		LOGGER.info( "HostString=" + connectionString + " User = " + String.valueOf(user));
		String[] hostPortArray = connectionString.split(":");
		
		if (hostPortArray.length < 2)
		{	
			hostPortArray = connectionString.split(";");
			if (hostPortArray.length < 2)
				return -1;
		}
		

		try
		{
			connectPort = Integer.parseInt(hostPortArray[1]);
			connectHost = InetAddress.getByName(hostPortArray[0]);
		} catch (UnknownHostException ex)
		{
			LOGGER.info( "Error parsing hostname: " + ex.getMessage());
			return -1;
		} catch (NumberFormatException nfe)
		{
			LOGGER.info( "Error parsing port: " + nfe.getMessage());
			return -1;
		}			

		LOGGER.info( "Hostname=" + connectHost.toString());
		LOGGER.info( "HostPort=" + connectPort);

		try
		{

			LOGGER.info( "Connecting to server");
			InetSocketAddress address = new InetSocketAddress(connectHost, connectPort);
			mySocket = new Socket();
			mySocket.connect(address,5000);
			LOGGER.info( "Socket created. Connected = " + mySocket.isConnected());
						
			if (!mySocket.isConnected())
			{
				Cleanup();
				return 2;
			
			}
			
			listening = true;
			StartListening();
			
			(new ClientXMLMessage(new Persister(),new ClientRequest("Authenticate",user+":"+password))).SendMessage(mySocket.getOutputStream());

			LOGGER.info( "Waiting for authentication result. Timeout in 15 ms");
			
			//Timeout
			new Thread(new Runnable()
			{
				public void run()
				{
					try
					{
						Thread.sleep(15000);
					} catch (InterruptedException e)
					{
						e.printStackTrace();
					}
					authenticationResultQueue.add(3);
				}
			}).start();
			
			Integer result = authenticationResultQueue.take();

			LOGGER.info( "Auth result = " + result);
			

			RequestData();
			
			if (result == 1)
				return 0;
			
			Shutdown();
			if (result == 3) //Timeout			
				return 2;						
			else
				return 3;
			
		}  catch (IllegalArgumentException illegalArg)
		{
			LOGGER.severe( "Illegal Arguments during connection :" + illegalArg);
			Cleanup();
			return -1;
		}

		catch (Exception e)
		{
			LOGGER.severe( "Connection error : " + e);
			Cleanup();
			return 2;
		}

	}

	// Synchronous shutdown
	public void Shutdown()
	{
		listening = false;
	}

	private void Cleanup()
	{
		listening = false;
		try
		{
			if (mySocket != null)
				mySocket.close();
			LOGGER.info( "Socket shutdown successfully.");
		} catch (IOException ioe)
		{
			LOGGER.severe( "Error closing socket " + ioe.getMessage());
		}
	}

}
