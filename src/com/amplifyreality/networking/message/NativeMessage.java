package com.amplifyreality.networking.message;

import android.util.Log;

import com.amplifyreality.networking.exceptions.UnknownClientActionException;
import com.amplifyreality.networking.model.ARObject;
import com.amplifyreality.networking.model.ClientRequest;

public class NativeMessage
{
	static String LOGTAG_NETWORKING = "AmplifyR-Networking_Java";
	
	public String action;
	public Object data;
	
	public NativeMessage(String action, Object data)
	{
		if (data == null) data = new String("nullData");
		Log.d(LOGTAG_NETWORKING, "Created native message. Action=" + action + ", Data=" + data.toString());
		this.action = action;
		this.data = data;
	}
	
	public Object GetXMLObject() throws UnknownClientActionException
	{
		if (action.equals("String"))
		{
			String dataString = data.toString();
			
			if (dataString.contains("RealmRequest:"))
			{
				
				String[] realmCode_Split = dataString.split(":");
				if (realmCode_Split.length < 2)
					throw new UnknownClientActionException("Invalid realm request. " + dataString);
				
				
				Log.i(LOGTAG_NETWORKING,"Requesting Realm with code="+ realmCode_Split[1]);
				ClientRequest request = new ClientRequest("Realm",realmCode_Split[1]);
				return request;
			}
			else if (dataString.contains("ResourceRequest:"))
			{
				String[] resourceName_Split = dataString.split(":");
				
				if (resourceName_Split.length < 2)
					throw new UnknownClientActionException("Invalid resource request request. " + dataString);
				
				String resourceName = resourceName_Split[1];
				
				Log.i(LOGTAG_NETWORKING,"Requesting resource with name="+ resourceName);
				ClientRequest request = new ClientRequest("Resource",resourceName);
				return request;
			}
			else if (dataString.contains("ObjectRepositioning:")) //[tag]:ObjectId=x;TX=1;TY=;TZ=;RX=...
			{
				String[] positionSplit = dataString.split(":");
				
				if (positionSplit.length < 2)
					throw new UnknownClientActionException("Invalid resource request request. " + dataString);
				
				return null;
			}
		}
		else if (action.equals("ARObjectUpdate"))
		{
			ARObject updateObject = (ARObject)data;
			Log.i(LOGTAG_NETWORKING, "Got ARObject update. ID=" + updateObject.Name);
			return updateObject;
		}
		throw new UnknownClientActionException();
	}
	
	
	
}