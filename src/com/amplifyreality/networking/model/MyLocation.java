package com.amplifyreality.networking.model;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Root;

//Non-Android version of android.location.Location class
@Root
public class MyLocation
{
	@Attribute
	public String Provider;
	@Attribute
    public long Time = 0;
	@Attribute
    public double Latitude = 0.0;
	@Attribute
    public double Longitude = 0.0;
	@Attribute
    public boolean HasAltitude = false;
	@Attribute
    public double Altitude = 0.0f;
	@Attribute
    public boolean HasSpeed = false;
	@Attribute
    public float Speed = 0.0f;
	@Attribute
    public boolean HasBearing = false;
	@Attribute
    public float Bearing = 0.0f;
	@Attribute
    public boolean HasAccuracy = false;
	@Attribute
    public float Accuracy = 0.0f;
}
