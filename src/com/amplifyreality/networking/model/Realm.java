package com.amplifyreality.networking.model;

import java.util.*;
import java.util.logging.Logger;

import org.simpleframework.xml.*;
import com.amplifyreality.util.Logging;

import android.util.Log;

//Represents a virtual, augmented world
@Root
public class Realm implements RealmPositionWatcher
{

	private final static Logger LOGGER = Logging.CreateLogger(Realm.class);

	public volatile boolean isUpdated = false;
	
	@Attribute
	public String Name;

	@Attribute
	public float QRUnitSize;
	
	@ElementMap
	public Map<String, ARObject> objectMap;

	List<RealmPositionWatcher> clients;
	
	public Realm()
	{
		
	}

	public Realm(String Name, float QRUnitSize)
	{
		this.QRUnitSize = QRUnitSize;
		this.Name = Name;
		objectMap = new HashMap<String, ARObject>();
	}

	public void AddClient(RealmPositionWatcher watcher)
	{
		clients.add(watcher);
	}
	

	@Override
	public void UpdateObject(String objectId, ARObject newObjectProperties)
	{
		if (!objectMap.containsKey(objectId))
		{
			LOGGER.info("Attempted to update non-existent object. Name="+ objectId);
			return;
		}
		
		ARObject arObject = objectMap.get(objectId);
			
		arObject.Update(newObjectProperties);	
		
		isUpdated = true;
		
		if (clients != null && clients.size() > 0)
			for (RealmPositionWatcher watcher : clients)
			{
				watcher.UpdateObject(objectId, newObjectProperties);
			}
	}
	
	@Override
	public String toString()
	{
		return Name + "=" + String.valueOf(objectMap);
	}
	
	public ArrayList getObjectList()
	{
		Log.i("AmplifyR-JNI","Returning ARObjects as array");
		ArrayList list = new ArrayList();
		for (ARObject value : objectMap.values())
		{
			list.add(value);
		}
		return list;
	}

}
