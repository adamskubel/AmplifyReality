package com.amplifyreality.networking.model;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Root;

@Root
public class RealmKey //implements Comparable<RealmKey> 
{
	@Attribute
	public String CodeValue;
	@Attribute(required=false)
	public double Latitude;
	@Attribute(required=false)
	public double Longitude;

	// Radius in meters
	@Attribute(required=false)
	public double Radius;
	
	public RealmKey()
	{	
	}
	
	public RealmKey(String code)
	{
		this.CodeValue = code;		
	}

	@Override
	public boolean equals(Object o)
	{
		if (o.getClass() != this.getClass())
			return false;
		RealmKey compareKey = (RealmKey) o;

		if (compareKey.CodeValue != null && compareKey.CodeValue.equals(CodeValue))
		{
			// Do location check
			if (Radius <= 0  || (Latitude == compareKey.Latitude && Longitude == compareKey.Longitude)) // This code doesn't use location or location is exact
				return true;
			if (isInRadius(Latitude, Longitude, compareKey.Latitude, compareKey.Longitude, Radius))
				return true;
			else
				return false;

		} else if (compareKey.CodeValue == null && CodeValue == null)
			return true;
		else
			return false;

	}

	public static boolean isInRadius(double lat0, double long0, double lat1, double long1, double radius)
	{
		return Calc(lat0,long0,lat1,long1) < radius;
	}

	//From http://www.codeproject.com/Articles/12269/Distance-between-locations-using-latitude-and-long
	public static double Calc(double Lat1, double Long1, double Lat2, double Long2)
	{
		/*
		 * The Haversine formula according to Dr. Math. http://mathforum.org/library/drmath/view/51879.html
		 * 
		 * dlon = lon2 - lon1 dlat = lat2 - lat1 a = (sin(dlat/2))^2 + cos(lat1) * cos(lat2) * (sin(dlon/2))^2 c = 2 * atan2(sqrt(a), sqrt(1-a)) d = R * c
		 * 
		 * Where dlon is the change in longitude dlat is the change in latitude c is the great circle distance in Radians. R is the radius of a spherical Earth. The locations of
		 * the two points in spherical coordinates (longitude and latitude) are lon1,lat1 and lon2, lat2.
		 */
		double dDistance = Double.MIN_VALUE;
		double dLat1InRad = Lat1 * (Math.PI / 180.0);
		double dLong1InRad = Long1 * (Math.PI / 180.0);
		double dLat2InRad = Lat2 * (Math.PI / 180.0);
		double dLong2InRad = Long2 * (Math.PI / 180.0);

		double dLongitude = dLong2InRad - dLong1InRad;
		double dLatitude = dLat2InRad - dLat1InRad;

		// Intermediate result a.
		double a = Math.pow(Math.sin(dLatitude / 2.0), 2.0) + Math.cos(dLat1InRad) * Math.cos(dLat2InRad) * Math.pow(Math.sin(dLongitude / 2.0), 2.0);

		// Intermediate result c (great circle distance in Radians).
		double c = 2.0 * Math.asin(Math.sqrt(a));

		// Distance.
		// const Double kEarthRadiusMiles = 3956.0;
		final Double kEarthRadiusKms = 6376.5;
		dDistance = kEarthRadiusKms * c;

		return dDistance*1000.0;
	}
	
	@Override
	public String toString()
	{
		return CodeValue + " (Lat=" + Latitude + ",Long="+ Longitude + ")";
	}

//	@Override
//	public int compareTo(RealmKey another)
//	{
//		if (this.equals(another))
//			return 0;
//		else
//			return 1;
//	}
}
