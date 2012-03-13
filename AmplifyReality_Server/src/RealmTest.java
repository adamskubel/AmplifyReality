import static org.junit.Assert.*;

import org.junit.Test;

import com.amplifyreality.networking.*;
import com.amplifyreality.networking.model.*;


public class RealmTest
{

	private static Realm CreateRealm1() throws Exception
	{

		Realm newRealm = new Realm(new RealmKey("TestRealm1"),3.0f);
		
		ARObject object1 = new ARObject();
		object1.Name = "TestObj1";
		object1.ModelName = "cube.obj";
		object1.Position = new Vector3(40,0,0);
		object1.Scale = new Vector3(15,10,15);
		object1.Rotation = new Vector3(0,0,0);
		object1.BoundingSphereRadius = 15;
		
		ARObject object2 = new ARObject();
		object2.Name = "TestObj2";
		object2.ModelName = "cube.obj";
		object2.Position = new Vector3(-40,0,0);
		object2.Scale = new Vector3(15,20,10);
		object2.Rotation = new Vector3(-1.1f,2.1f,3.1f);
		object2.BoundingSphereRadius = 18;
		
		
		newRealm.objectMap.put(object1.Name,object1);
		newRealm.objectMap.put(object2.Name,object2);
		
		return newRealm;
	}
	
	@Test
	public void RealmStorageTest_NoLocation() throws Exception
	{
		RealmManager manager = new RealmManager();
		Realm testRealm1 = CreateRealm1();
		
		manager.AddRealm(testRealm1);
		
		RealmKey key = new RealmKey();
		key.CodeValue = "TestRealm1";
		manager.RequestRealm(key);
		
		assertEquals(1,manager.NumRealms());		
	}
	
	@Test
	public void RealmStorageTest_WithLocation() throws Exception
	{
		RealmManager manager = new RealmManager();
		Realm testRealm1 = CreateRealm1();
		
		manager.AddRealm(testRealm1);
		
		RealmKey key = new RealmKey();
		key.CodeValue = "TestRealm1";
		key.Longitude = 80;
		key.Latitude = 40;
		manager.RequestRealm(key);
		
		assertEquals(1,manager.NumRealms());		
	}
	
	@Test
	public void RealmKeyEquivalance_NoLocationBoth() throws Exception
	{
		RealmKey key1 = new RealmKey("TestRealm1");
		RealmKey key2 = new RealmKey("TestRealm1");
		assertEquals(key1, key2);
	}
	
	@Test
	public void RealmKeyEquivalance_NoLocation_One() throws Exception
	{
		RealmKey key1 = new RealmKey("TestRealm1");
		RealmKey key2 = new RealmKey("TestRealm1");
		key2.Latitude = -40;
		key2.Longitude = 80;
		key2.Radius = 10;
		assertTrue(key1.equals(key2));
		assertFalse(key2.equals(key1));
	}
	
}
