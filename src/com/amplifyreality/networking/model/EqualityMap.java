package com.amplifyreality.networking.model;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

//Makes an array list look like a map. lol.
public class EqualityMap<K, V> implements Map<K, V>
{
	ArrayList<V> valueList;
	ArrayList<K> keyList;

	public EqualityMap()
	{
		valueList = new ArrayList<V>();
		keyList = new ArrayList<K>();
	}

	@Override
	public void clear()
	{
		keyList.clear();
		valueList.clear();
	}

	@Override
	public boolean containsKey(Object key)
	{
		return keyList.contains(key);
	}

	@Override
	public boolean containsValue(Object value)
	{
		return valueList.contains(value);
	}

	@Override
	public Set<java.util.Map.Entry<K, V>> entrySet()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public V get(Object key)
	{
		return valueList.get(keyList.indexOf(key));
	}

	@Override
	public boolean isEmpty()
	{
		return keyList.isEmpty();
	}

	@Override
	public Set<K> keySet()
	{
		return new HashSet<K>(keyList);
	}

	@Override
	public V put(K key, V value)
	{
		int index = keyList.indexOf(key);
		if (index != -1)
		{
			V lastValue = valueList.get(index);
			valueList.set(index, value);
			return lastValue;
		} else
		{
			keyList.add(key);
			valueList.add(value);
			return null;
		}
	}

	@Override
	public void putAll(Map<? extends K, ? extends V> m)
	{
		for (K key : m.keySet())
		{
			put(key, m.get(key));
		}
	}

	@Override
	public V remove(Object key)
	{
		int index = keyList.indexOf(key);
		if (index != -1)
		{
			V lastValue = valueList.get(index);
			return lastValue;
		} else
			return null;

	}

	@Override
	public int size()
	{
		return keyList.size();
	}

	@Override
	public Collection<V> values()
	{
		return valueList;
	}

}