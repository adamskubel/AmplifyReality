package com.amplifyreality.networking.model;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

@Root
public class ClientRequest
{
	
	public ClientRequest()
	{
		RequestType="";
		RequestData ="";
	}
	
	@Attribute
	public String RequestType;
	
	@Attribute
	public String RequestData;
	
	@Element(required=false)
	public MyLocation CurrentLocation;
	
	public ClientRequest(String requestType, String requestData)
	{
		RequestType = requestType;
		RequestData = requestData;
	}
	
	
}
