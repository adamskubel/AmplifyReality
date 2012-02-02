package com.amplifyreality.networking.model;

public class DataHeader
{
	public String XmlDataType;
	public int NumBytes;
	
	public DataHeader(String XmlDataType, int NumBytes)
	{
		this.XmlDataType = XmlDataType;
		this.NumBytes = NumBytes;
	}
}
