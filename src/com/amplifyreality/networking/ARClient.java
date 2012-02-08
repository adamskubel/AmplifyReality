package com.amplifyreality.networking;

import java.io.*;
import java.net.Socket;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import com.amplifyreality.AmplifyRealityActivity;
import com.amplifyreality.networking.exceptions.InvalidHeaderMessageException;
import com.amplifyreality.networking.exceptions.UnknownClientActionException;
import com.amplifyreality.networking.message.ClientMessage;
import com.amplifyreality.networking.message.ClientXMLMessage;
import com.amplifyreality.networking.message.NativeMessage;
import com.amplifyreality.networking.model.ARObject;
import com.amplifyreality.networking.model.Realm;
import com.amplifyreality.networking.model.DataHeader;
import com.amplifyreality.networking.model.WavefrontObj;

import android.util.Log;
import java.util.concurrent.LinkedBlockingQueue;

public class ARClient
{

	static String LOGTAG_NETWORKING = "AmplifyR-Networking_Java";

	String host = "192.168.1.9";
	int port = 12312;
	volatile boolean listening = false;

	private LinkedBlockingQueue<Object[]> outgoingQueue;

	Socket mySocket;
	BufferedReader reader;

	public ARClient()
	{
		reader = null;
		outgoingQueue = new LinkedBlockingQueue<Object[]>();
		Connect();

	}

	public ARClient(String host, int port)
	{
		this.host = host;
		this.port = port;
		reader = null;
		Connect();
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
		outgoingQueue.add(msgs);
	}

	private void RequestData()
	{
		Thread t = new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				while (listening)
				{
					Object[] msgs = null;
					try
					{

						// msgs = outgoingQueue.take();
						msgs = AmplifyRealityActivity.GetOutgoingMessages();
					}
					// catch (InterruptedException e)
					// {
					// Log.e(LOGTAG_NETWORKING,"Interrupted taking data from outgoing queue.",e);
					// }
					catch (Exception e)
					{
						Log.e(LOGTAG_NETWORKING, "Exception checking native layer", e);
						continue;
					}
					
					//Process messages
					SendNativeMessages(msgs);
					
					
					try
					{
						Thread.sleep(5000);
					} catch (InterruptedException e)
					{
						Log.e(LOGTAG_NETWORKING, "Interrupted while sleeping.", e);
					}
				}
				Cleanup();
			}

		});
		t.start();
	}

	private void StartListening()
	{
		listening = true;
		Thread t = new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				// Sleep a bit to ensure native layer has started
				try
				{
					Thread.sleep(6000);
				} catch (InterruptedException e1)
				{
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
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
								AmplifyRealityActivity.OnMessage(inputLine, null); // Process as string
							}
							// Log.i(LOGTAG_NETWORKING, "Server says: " + inputLine);
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
		Log.i(LOGTAG_NETWORKING, "Received " + result + " bytes from server.");
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
				if (className.equals(ARObject.class.getCanonicalName()))
				{
					ARObject arObject = serializer.read(ARObject.class, data);
					Log.i(LOGTAG_NETWORKING, "Received ARObject: " + arObject.toString());
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

	private void Connect()
	{
		Thread t = new Thread(new Runnable()
		{
			@Override
			public void run() 
			{
				try
				{
					Log.i(LOGTAG_NETWORKING, "Connecting to server");
					mySocket = new Socket(host, port);
					Log.i(LOGTAG_NETWORKING, "Socket created");
	
					RequestData();
					StartListening();
	
	
				} catch (Exception e)
				{
					Log.e(LOGTAG_NETWORKING, "Connection error", e);
					Cleanup();
				}
			}
		});
		t.start();
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
			if (mySocket != null)
				mySocket.close();
			Log.i(LOGTAG_NETWORKING, "Socket shutdown successfully.");
		} catch (IOException ioe)
		{
			Log.e(LOGTAG_NETWORKING, "Error closing socket", ioe);
		}
	}

}
