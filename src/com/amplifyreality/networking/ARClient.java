package com.amplifyreality.networking;

import java.io.*;
import java.net.Socket;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import com.amplifyreality.AmplifyRealityActivity;
import com.amplifyreality.networking.model.ARObject;
import com.amplifyreality.networking.model.Realm;
import com.amplifyreality.networking.model.DataHeader;

import android.os.Handler;
import android.util.Log;

public class ARClient
{

	static String LOGTAG_NETWORKING = "AmplifyR-Networking_Java";

	String host = "192.168.1.9";
	int port = 12312;
	volatile boolean listening = false;
	volatile boolean needsData = true;

	Socket mySocket;
	BufferedReader reader;

	public ARClient()
	{
		reader = null;
		Connect();
	}

	public ARClient(String host, int port)
	{
		this.host = host;
		this.port = port;
		reader = null;
		Connect();
	}

	private void RequestData()
	{
		Thread t = new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				try
				{
					PrintWriter writer = new PrintWriter(mySocket.getOutputStream());

					while (listening)
					{
					//	if (needsData)
						{
							writer.write("JoinWorld\n");
							writer.flush();
							Log.d(LOGTAG_NETWORKING, "Sent message to server. Sleeping 2sec");
						}
						Thread.sleep(5000);
					}

				} catch (IOException e)
				{
					Shutdown();
				} catch (InterruptedException e)
				{
					Log.e(LOGTAG_NETWORKING, "Thread error in RequestData()", e);
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
				DataHeader dataHeader = null;
				try
				{
					BufferedReader reader = new BufferedReader(new InputStreamReader(mySocket.getInputStream()));
					while (listening)
					{
						Log.i(LOGTAG_NETWORKING, "Waiting for line");
						if (dataHeader == null)
						{
							String inputLine = reader.readLine();
							if (inputLine == null)
							{
								Shutdown();
								break;
							}
							dataHeader = GetHeader(inputLine);

							AmplifyRealityActivity.OnMessage(inputLine);
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

	private DataHeader GetHeader(String inputLine)
	{
		if (inputLine.startsWith("BYTES_NEXT:"))
		{
			try
			{
				int parseResult = Integer.valueOf(inputLine.split(":")[1]);
				if (parseResult > 0)
				{
					return new DataHeader(null, parseResult);
				}
			} catch (NumberFormatException nfe)
			{
				return null;
			}
		} else if (inputLine.startsWith("XML_BYTES_NEXT:"))
		{
			try
			{
				if (inputLine.split(":").length >= 2)
				{
					int parseResult = Integer.valueOf(inputLine.split(":")[1]);
					String xmlName = inputLine.split(":")[2];
					if (parseResult > 0 && xmlName.length() > 0)
					{
						return new DataHeader(xmlName, parseResult);
					}
				}
			} catch (NumberFormatException nfe)
			{
				return null;
			}
		}
		return null;
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
			AmplifyRealityActivity.OnMessage(data);
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
					needsData = false;
				}
			} catch (Exception e)
			{
				Log.e(LOGTAG_NETWORKING, "Error parsing message from server", e);
			}
		}

	}

	private void Connect()
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
