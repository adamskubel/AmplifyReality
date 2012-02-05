package com.amplifyreality.networking.message;

import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;

public class ClientStringMessage implements ClientMessage
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