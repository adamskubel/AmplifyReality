package com.amplifyreality.networking;

import java.io.*;
import java.net.Socket;

import com.amplifyreality.AmplifyRealityActivity;

import android.util.Log;

public class ARClient
{
	
	
	
	static String LOGTAG_NETWORKING = "AmplifyR-Networking_Java";

	String host = "192.168.1.9";
	int port = 12312;
	volatile boolean listening = false;

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
						writer.write("DefaultWorld\n");
						writer.flush();
						Log.d(LOGTAG_NETWORKING, "Sent message to server. Sleeping 2sec");
						Thread.sleep(2000);
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
				int expectBytes = 0;
				try
				{
					BufferedReader reader = new BufferedReader(new InputStreamReader(mySocket.getInputStream()));
					while (listening)
					{
						Log.i(LOGTAG_NETWORKING, "Waiting for line");
						if (expectBytes <= 0)
						{
							String inputLine = reader.readLine();
							if (inputLine == null)
							{
								Shutdown();
								break;
							} else if (inputLine.startsWith("BYTES_NEXT:"))
							{
								try
								{
									int parseResult = Integer.valueOf(inputLine.split(":")[1]);
									if (parseResult > 0)
										expectBytes = parseResult;
								} catch (NumberFormatException nfe)
								{
									expectBytes = 0;
								}
							}
							AmplifyRealityActivity.OnMessage(inputLine);
							Log.i(LOGTAG_NETWORKING, "Server says: " + inputLine);
						}
						else
						{
							char buffer[] = new char[expectBytes];
							int result = reader.read(buffer, 0, expectBytes);
							Log.i(LOGTAG_NETWORKING,"Received " + result + " bytes from server.");
							String data = new String(buffer);
						//Log.i(LOGTAG_NETWORKING,"Data is" + data);
							AmplifyRealityActivity.OnMessage(data);
							expectBytes = 0;
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
