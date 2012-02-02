package com.amplifyreality.networking.model;

import java.util.*;

import org.simpleframework.xml.*;

import com.amplifyreality.networking.RealmPositionWatcher;
import com.amplifyreality.networking.model.*;

//Represents a virtual, augmented world
@Root
public class Realm implements RealmPositionWatcher
{

	@Attribute
	public String Name;

	@ElementMap
	public Map<String, ARObject> objectMap;

	List<RealmPositionWatcher> clients;

	public Realm()
	{
		objectMap = new HashMap<String, ARObject>();
	}

	public void AddClient(RealmPositionWatcher watcher)
	{
		clients.add(watcher);
	}
	
	public void RemoteClient(RealmPositionWatcher watcher)
	{
		
	}

	@Override
	public void PositionUpdate(String objectId, Vector3 newPosition)
	{
		ARObject arObject = objectMap.get(objectId);
		if (arObject == null)
			return;
		
		arObject.Position = newPosition;
		
		for (RealmPositionWatcher watcher : clients)
		{
			watcher.PositionUpdate(objectId, newPosition);
		}
	}

	@Override
	public void RotationUpdate(String objectId, Vector3 newRotation)
	{
		ARObject arObject = objectMap.get(objectId);
		if (arObject == null)
			return;
		
		arObject.Rotation = newRotation;
		
		for (RealmPositionWatcher watcher : clients)
		{
			watcher.RotationUpdate(objectId, newRotation);
		}
	}

	@Override
	public void ScaleUpdate(String objectId, Vector3 newScale)
	{
		ARObject arObject = objectMap.get(objectId);
		if (arObject == null)
			return;
		
		arObject.Scale = newScale;
		
		for (RealmPositionWatcher watcher : clients)
		{
			watcher.ScaleUpdate(objectId, newScale);
		}
	}
	
	@Override
	public String toString()
	{
		return Name + "=" + String.valueOf(objectMap);
	}

}
