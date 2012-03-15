package com.amplifyreality.networking.message;

import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.StringWriter;

import org.simpleframework.xml.Serializer;


public class ClientXMLMessage implements ClientMessage
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
			bufferedStringWriter.append('\n');
			byte[] bytes = stringWriter.toString().getBytes();
							
			BufferedOutputStream bufferedOut = new BufferedOutputStream(output);
			bufferedOut.flush();
			String header ="XML_BYTES_NEXT:" + bytes.length + ":" + data.getClass().getCanonicalName() + "\n";
			bufferedOut.write(header.getBytes());
			bufferedOut.write(bytes);
			bufferedOut.flush();				
		} catch (Exception e)
		{
			throw new IOException(e.getMessage());
		}			
	}	
	
}