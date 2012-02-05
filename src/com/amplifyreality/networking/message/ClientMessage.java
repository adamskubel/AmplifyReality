package com.amplifyreality.networking.message;

import java.io.IOException;
import java.io.OutputStream;

public interface ClientMessage
{
	public void SendMessage(OutputStream output) throws IOException;
}
