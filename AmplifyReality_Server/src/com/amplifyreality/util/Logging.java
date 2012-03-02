package com.amplifyreality.util;

import java.util.logging.ConsoleHandler;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.LogRecord;
import java.util.logging.Logger;

import android.text.format.Formatter;

public class Logging
{
	public static Logger CreateLogger(Class clazz)
	{
		Logger LOGGER = Logger.getLogger(clazz.getName());
		
		Handler[] handlers = LOGGER.getHandlers();
		for (int i=0;i<handlers.length;i++)
			LOGGER.removeHandler(handlers[i]);
		
		LOGGER.setUseParentHandlers(false);
		try
		{
			LOGGER.addHandler(new Handler()
			{
				@Override
				public void publish(LogRecord arg0)
				{
					if (arg0.getLevel() == Level.SEVERE || arg0.getLevel() == Level.WARNING)
						System.err.println(arg0.getLevel().getName() + ":" + arg0.getMessage());
					else 
						System.out.println(arg0.getLevel().getName() + ":" + arg0.getMessage());
					
					if (arg0.getThrown() != null)
						arg0.getThrown().printStackTrace(System.err);
					
					return;
				}
				
				@Override
				public java.util.logging.Formatter getFormatter()
				{
					return null;
				}

				@Override
				public void flush(){
					return;
				}

				@Override
				public void close() throws SecurityException {
					return;
				}
			});
		} catch (Exception e)
		{
			e.printStackTrace();
		}
		return LOGGER;
	}
}
