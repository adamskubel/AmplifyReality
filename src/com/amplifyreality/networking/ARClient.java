package com.amplifyreality.networking;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.concurrent.LinkedBlockingQueue;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import android.location.Location;
import android.util.Log;

import com.amplifyreality.AmplifyRealityActivity;
import com.amplifyreality.LocationCollector;
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


public class ARClient
{

	static String LOGTAG_NETWORKING = "AmplifyR-Networking_Java";

	//Defaults
	String host = "192.168.1.9";
	int port = 12312;
	
	volatile boolean listening = false;

	private LinkedBlockingQueue<Object[]> outgoingQueue;
	private LinkedBlockingQueue<Integer> authenticationResultQueue;
	
	private LocationCollector locationCollector;

	Socket mySocket;
	BufferedReader reader;

	public ARClient(LocationCollector locationCollector)
	{
		this.locationCollector = locationCollector;
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

			if (message.isLocationSensitive() && locationCollector != null)
			{
				Location location = locationCollector.getLocation();
				message.setLocation(location);
				Log.d("AmplifyR-GPS","Setting message location:" + location);
			}
			
			try
			{
				ClientXMLMessage msg = new ClientXMLMessage(xmlSerializer, message.GetXMLObject());
				Log.i(LOGTAG_NETWORKING, "Sending nativemsg: " + message.action);
				try
				{
					msg.SendMessage(mySocket.getOutputStream());
				} catch (IOException e)
				{
					Log.e(LOGTAG_NETWORKING, "Error sending message.", e);
				}
			} catch (UnknownClientActionException e)
			{
				Log.w(LOGTAG_NETWORKING, "Unknown client action '" + message.action + "'");
			}

		}
	}

	public void QueueNativeMessages(Object[] msgs)
	{
		Log.d(LOGTAG_NETWORKING,"Native layer added " + msgs.length + " messages.");
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
						Log.e(LOGTAG_NETWORKING,"Error taking from queue",e);
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
		Log.i(LOGTAG_NETWORKING, "Waiting for server messages");
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
								
								Log.d(LOGTAG_NETWORKING,"Inputline=" + inputLine);
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
					Log.e(LOGTAG_NETWORKING, "IO error", e);
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
		Log.i(LOGTAG_NETWORKING, "Received " + result + " bytes from server as expected.");
		while (result < dataHeader.NumBytes)
		{
			Log.i(LOGTAG_NETWORKING, "Received " + result + " bytes from server. Expected " + dataHeader.NumBytes + " bytes. Sleeping for a bit.");
			try
			{
				Thread.sleep(1000);
			} catch (InterruptedException e)
			{
				Log.e(LOGTAG_NETWORKING,"Error during wait message sleep",e);
			}
			result = bufferedReader.read(buffer, result, dataHeader.NumBytes);
		}

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
					Log.i(LOGTAG_NETWORKING, "Received ARObject: " + arObject.toString());
					AmplifyRealityActivity.OnMessage("ARObjectUpdate",arObject);					
				} else if (className.equals(Realm.class.getCanonicalName()))
				{
					Realm realm = serializer.read(Realm.class, data);
					Log.i(LOGTAG_NETWORKING, "Received new Realm: " + realm.toString());
					AmplifyRealityActivity.OnMessage("RealmObject", realm);
				} else if (className.equals(WavefrontObj.class.getCanonicalName()))
				{
					WavefrontObj wavefrontObj = serializer.read(WavefrontObj.class, data);
					Log.i(LOGTAG_NETWORKING, "Received new wavefront model: " + wavefrontObj.toString());
					AmplifyRealityActivity.OnMessage("WavefrontObject", wavefrontObj);
				} else
				{
					Log.w(LOGTAG_NETWORKING, "Received unknown XML object. Classname=" + className);
				}
			} catch (Exception e)
			{
				Log.e(LOGTAG_NETWORKING, "Error parsing message from server", e);
			}
		}

	}

	public int Connect(String connectionString, String user, String password)
	{
		if (listening)
			return 2;
		
		InetAddress connectHost = null;
		int connectPort = 0;
		Log.i(LOGTAG_NETWORKING, "HostString=" + connectionString + " User = " + String.valueOf(user));
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
			Log.i(LOGTAG_NETWORKING, "Error parsing hostname", ex);
			return -1;
		} catch (NumberFormatException nfe)
		{
			Log.i(LOGTAG_NETWORKING, "Error parsing port", nfe);
			return -1;
		}			

		Log.i(LOGTAG_NETWORKING, "Hostname=" + connectHost.toString());
		Log.i(LOGTAG_NETWORKING, "HostPort=" + connectPort);

		try
		{

			Log.i(LOGTAG_NETWORKING, "Connecting to server");
			InetSocketAddress address = new InetSocketAddress(connectHost, connectPort);
			mySocket = new Socket();
			mySocket.connect(address,5000);
			mySocket.setReceiveBufferSize(250000);
			Log.i(LOGTAG_NETWORKING, "Socket created. Connected = " + mySocket.isConnected());
						
			if (!mySocket.isConnected())
			{
				Cleanup();
				return 2;
			
			}
			
			listening = true;
			StartListening();
			
			(new ClientXMLMessage(new Persister(),new ClientRequest("Authenticate",user+":"+password))).SendMessage(mySocket.getOutputStream());

			Log.i(LOGTAG_NETWORKING, "Waiting for authentication result. Timeout in 15 ms");
			
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

			Log.i(LOGTAG_NETWORKING, "Auth result = " + result);
			

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
			Log.e(LOGTAG_NETWORKING, "Illegal Arguments during connection", illegalArg);
			Cleanup();
			return -1;
		}

		catch (Exception e)
		{
			Log.e(LOGTAG_NETWORKING, "Connection error", e);
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
			Log.i(LOGTAG_NETWORKING, "Socket shutdown successfully.");
		} catch (IOException ioe)
		{
			Log.e(LOGTAG_NETWORKING, "Error closing socket", ioe);
		}
	}

}
