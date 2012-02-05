package com.amplifyreality.networking.message;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStream;

public class ClientByteMessage implements ClientMessage
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
		String byteHeader = "BYTES_NEXT:" + byteMessage.length + "\n";
		bufferedOut.write(byteHeader.getBytes());
		bufferedOut.flush();
		bufferedOut.write(byteMessage);
		bufferedOut.flush();
	}
}
