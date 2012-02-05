package com.amplifyreality.networking.model;

import com.amplifyreality.networking.exceptions.InvalidHeaderMessageException;

public class DataHeader
{
	public String XmlDataType;
	public int NumBytes;
	
	public DataHeader(String XmlDataType, int NumBytes)
	{
		this.XmlDataType = XmlDataType;
		this.NumBytes = NumBytes;
	}
	
	public static DataHeader CreateHeaderFromMessage(String inputLine) throws InvalidHeaderMessageException
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
				throw new InvalidHeaderMessageException();
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
				throw new InvalidHeaderMessageException();
			}
		}
		throw new InvalidHeaderMessageException();
	}

}
