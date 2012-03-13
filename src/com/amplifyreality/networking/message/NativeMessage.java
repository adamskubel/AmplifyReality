package com.amplifyreality.networking.message;

import android.location.Location;
import android.util.Log;

import com.amplifyreality.networking.exceptions.UnknownClientActionException;
import com.amplifyreality.networking.model.ARObject;
import com.amplifyreality.networking.model.ClientRequest;
import com.amplifyreality.networking.model.MyLocation;

public class NativeMessage
{
	static String LOGTAG_NETWORKING = "AmplifyR-Networking_Java";

	public String action;
	public Object data;

	MyLocation newLocation;
	
	public NativeMessage(String action, Object data)
	{
		if (data == null)
			data = new String("nullData");
		Log.d(LOGTAG_NETWORKING, "Created native message. Action=" + action + ", Data=" + data.toString());
		this.action = action;
		this.data = data;
		newLocation = null;
	}

	public boolean isLocationSensitive()
	{
		if (action.equals("String"))
		{
			String dataString = data.toString();
			if (dataString.contains("RealmRequest:"))
			{
				return true;
			}
		}
		return false;
	}

	public void setLocation(Location androidLocation)
	{
		//Copy location to XML structure
		newLocation = new MyLocation();
		newLocation.Accuracy = androidLocation.getAccuracy();
		newLocation.HasAccuracy = androidLocation.hasAccuracy();
		newLocation.Altitude = androidLocation.getAltitude();
		newLocation.HasAccuracy = androidLocation.hasAltitude();
		newLocation.Bearing = androidLocation.getBearing();
		newLocation.HasBearing = androidLocation.hasBearing();
		newLocation.Latitude = androidLocation.getLatitude();
		newLocation.Longitude = androidLocation.getLongitude();
		newLocation.Provider = androidLocation.getProvider();
		newLocation.Speed = androidLocation.getSpeed();
		newLocation.HasSpeed = androidLocation.hasSpeed();
		newLocation.Time = androidLocation.getTime();
		
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

				Log.i(LOGTAG_NETWORKING, "Requesting Realm with code=" + realmCode_Split[1]);
				ClientRequest request = new ClientRequest("Realm", realmCode_Split[1]);
				if (newLocation != null)
					request.CurrentLocation = newLocation;
				return request;
			} else if (dataString.contains("ResourceRequest:"))
			{
				String[] resourceName_Split = dataString.split(":");

				if (resourceName_Split.length < 2)
					throw new UnknownClientActionException("Invalid resource request request. " + dataString);

				String resourceName = resourceName_Split[1];

				Log.i(LOGTAG_NETWORKING, "Requesting resource with name=" + resourceName);
				ClientRequest request = new ClientRequest("Resource", resourceName);
				if (newLocation != null)
					request.CurrentLocation = newLocation;
				return request;
			}
		} else if (action.equals("ARObjectUpdate"))
		{
			ARObject updateObject = (ARObject) data;
			Log.i(LOGTAG_NETWORKING, "Got ARObject update. ID=" + updateObject.Name);
			return updateObject;
		}
		throw new UnknownClientActionException();
	}

}