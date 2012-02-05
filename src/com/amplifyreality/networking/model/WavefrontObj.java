package com.amplifyreality.networking.model;

import org.simpleframework.xml.*;
import org.simpleframework.xml.core.*;

@Root
public class WavefrontObj
{
	@Element
	public String Name;
	
	@Element(data = true)
	public String ObjData;
	
	public WavefrontObj()
	{
		Name = "";
		ObjData = "";
	}
	
	public WavefrontObj(String Name, String ObjData)
	{
		this.Name = Name;
		this.ObjData = ObjData;
	}
}
