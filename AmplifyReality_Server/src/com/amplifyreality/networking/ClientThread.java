package com.amplifyreality.networking;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.Socket;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.logging.*;
import org.simpleframework.xml.*;
import org.simpleframework.xml.core.*;

import com.amplifyreality.networking.model.Realm;

public class ClientThread implements MessageListener
{

	private interface ClientMessage
	{
		public void SendMessage(OutputStream output) throws IOException;
	}

	private class ClientByteMessage implements ClientMessage
	{
		private byte[] byteMessage;
		private String stringMessage;

		public ClientByteMessage(byte[] byteMessage)
		{
			this.byteMessage = byteMessage;
		}

		public void SendMessage(OutputStream output) throws IOException
		{

			BufferedOutputStream bufferedOut = new BufferedOutputStream(output);
			LOGGER.info("Sending " + byteMessage.length + " bytes to client");
			String byteHeader = "BYTES_NEXT:" + byteMessage.length + "\n";
			bufferedOut.write(byteHeader.getBytes());
			bufferedOut.flush();
			bufferedOut.write(byteMessage);
			bufferedOut.flush();
		}
	}

	private class ClientStringMessage implements ClientMessage
	{
		private String stringMessage;

		public ClientStringMessage(String stringMessage)
		{
			this.stringMessage = stringMessage;
		}

		public void SendMessage(OutputStream output) throws IOException
		{

			PrintWriter writer = new PrintWriter(output);
			writer.write(stringMessage + "\n");
			writer.flush();

		}
	}
	
	private class ClientXMLMessage implements ClientMessage
	{
		private Serializer serializer;
		private Object data;
		
		public ClientXMLMessage(Serializer serializer, Object data)
		{
			this.serializer = serializer;
			this.data = data;
		}
		
		@Override
		public void SendMessage(OutputStream output) throws IOException
		{
			try
			{
				StringWriter stringWriter = new StringWriter();
				BufferedWriter bufferedStringWriter = new BufferedWriter(stringWriter);
				serializer.write(data,bufferedStringWriter);
				byte[] bytes = stringWriter.toString().getBytes();
								
				BufferedOutputStream bufferedOut = new BufferedOutputStream(output);
				String header ="XML_BYTES_NEXT:" + bytes.length + ":" + data.getClass().getCanonicalName() + "\n";
				bufferedOut.write(header.getBytes());
				bufferedOut.flush();
				bufferedOut.write(bytes);
				bufferedOut.flush();				
			} catch (Exception e)
			{
				throw new IOException(e.getMessage());
			}			
		}	
		
	}

	volatile boolean listening;
	Socket clientSocket;

	// Messages from remote client
	LinkedBlockingQueue<String> remoteMessageQueue;

	// Messages from other clients
	LinkedBlockingQueue<String> messageInputQueue;

	// Messages to this client
	LinkedBlockingQueue<ClientMessage> messageOutputQueue;

	private final static Logger LOGGER = Logger.getLogger(ClientThread.class.getName());
	static
	{
		LOGGER.addHandler(new StreamHandler(System.out, new SimpleFormatter()));
		try
		{
			LOGGER.addHandler(new FileHandler("ARServer.log"));
		} catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	private enum ClientStates
	{
		WaitingForCode, Active;
	}

	private ClientStates clientState = ClientStates.WaitingForCode;

	public ClientThread(Socket _clientSocket)
	{
		messageInputQueue = new LinkedBlockingQueue<String>();
		messageOutputQueue = new LinkedBlockingQueue<ClientMessage>();
		remoteMessageQueue = new LinkedBlockingQueue<String>();

		listening = true;
		clientSocket = _clientSocket;

		LOGGER.info("Created new client. Host=" + clientSocket.getInetAddress().toString());

		// Listen for messages, and put them on the remote message queue as they arrive
		Thread listenThread = new Thread(new Runnable()
		{

			@Override
			public void run()
			{
				try
				{
					BufferedReader input = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
					while (listening)
					{
						String text = input.readLine();
						LOGGER.fine("SocketListenerThread: Message received: " + text);
						if (text == null)
						{
							Shutdown();
							break;
						}

						remoteMessageQueue.add(text);
					}

				} catch (IOException e)
				{
					e.printStackTrace();
					listening = false;
				}

				Cleanup();
			}

		});

		// Process messages arriving on the remote message queue
		Thread remoteMessageProcessing = new Thread(new Runnable()
		{

			@Override
			public void run()
			{
				while (listening)
				{
					try
					{
						LOGGER.info("RemoteMessageProcessorThread: Waiting for remote messages from client.");
						// Block until a message is available to process
						String message = remoteMessageQueue.take();
						ProcessRemoteMessage(message);

					} catch (InterruptedException e)
					{
						LOGGER.severe("RemoteMessageProcessorThread: Interrupted while taking message from queue");
						messageInputQueue.clear();
						Shutdown();
					}
				}
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
						LOGGER.info("LocalMessageProcessorThread: Waiting for messages on queue");
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
						LOGGER.info("RemoteClientSenderThread: Waiting for message on queue to send");
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
		remoteMessageProcessing.start();
		messageProcessing.start();
		sendMessages.start();

	}

	private void ProcessRemoteMessage(String message)
	{

		String action = message;
		
		if (action.equals("SendObjectFile"))
		{
			//Resolve file name here
			byte[] data = RealmManager.getInstance().GetObjFile("filename");
			if (data != null)
			{
				messageOutputQueue.add(new ClientByteMessage(data));
			}
		}
		else if (action.equals("JoinWorld"))
		{
			try
			{
				Realm realm = RealmManager.getInstance().RequestRealm("TestRealm.xml");
				Serializer serial = new Persister();				
				messageOutputQueue.add(new ClientXMLMessage(serial,realm));			
			} catch (Exception e)
			{
				LOGGER.log(Level.WARNING,"Error retrieving realm.",e);
				//Notify client of failure
			}			
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
			System.out.println("Socket shutdown successfully.");
		} catch (IOException ioe)
		{
			ioe.printStackTrace();
			System.out.println("Error closing socket");
		}
	}

}
