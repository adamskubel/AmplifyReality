package com.amplifyreality.test;

public class TestClient
{
	public static void main(String[] args)
	{
		ARClientMock mock = new ARClientMock();
		mock.Connect("192.168.1.9:12312", "test", "test");
		
	}
}
