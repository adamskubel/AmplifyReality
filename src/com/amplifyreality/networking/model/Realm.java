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
	public float QRUnitSize;

	@ElementMap
	public HashMap<String, ARObject> objectMap;

	@Element
	public RealmKey Key;

	List<RealmPositionWatcher> clients;

	public Realm()
	{

	}

	public Realm(RealmKey Key, float QRUnitSize)
	{
		this.QRUnitSize = QRUnitSize;
		this.Key = Key;
		objectMap = new HashMap<String, ARObject>();
	}

	public void AddClient(RealmPositionWatcher watcher)
	{
		if (clients == null)
			clients = new ArrayList<RealmPositionWatcher>();
		clients.add(watcher);
	}

	public void AddNewObject(String objectId, ARObject newObject)
	{
		if (objectMap.containsKey(objectId))
		{
			LOGGER.info("Attempted to create existing object. Name=" + objectId + ". Processing as update.");
			UpdateObject(objectId, newObject);
		}

		objectMap.put(objectId, newObject);
	}
	
	public String getCodeName()
	{
		return Key.CodeValue;
	}

	@Override
	public void UpdateObject(String objectId, ARObject newObjectProperties)
	{
		if (!objectMap.containsKey(objectId))
		{
			LOGGER.info("Attempted to update non-existent object. Name=" + objectId);
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
		return Key.CodeValue + "=" + String.valueOf(objectMap);
	}

	public ArrayList getObjectList()
	{
		Log.i("AmplifyR-JNI", "Returning ARObjects as array. Size = " + objectMap.size());
		if (!objectMap.isEmpty())
		{
			ArrayList list = new ArrayList();
			for (ARObject value : objectMap.values())
			{
				list.add(value);
			}
			return list;
		}
		return null;
	}

}
