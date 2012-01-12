#include "datacollection/SensorCollector.hpp"


SensorCollector::SensorCollector(ASensorManager* sensorManager, ALooper * looper)
{
	accelSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_ACCELEROMETER);
	gyroSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_GYROSCOPE);
	magnetoSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_MAGNETIC_FIELD);

	sensorQueue = ASensorManager_createEventQueue(sensorManager,looper, LOOPER_ID_USER, NULL, NULL);

	accelEnabled = false;
	gyroEnabled = false;
	magEnabled = false;
	resetNextTime = false;

	rotationVector = cv::Mat::zeros(1,3,CV_64F);
}

SensorCollector::~SensorCollector()
{
	DisableSensors();

	/*delete accelSensor;
	delete magnetoSensor;
	delete gyroSensor;*/

	LOGI(LOGTAG_SENSOR,"Sensors Deleted");
}


void SensorCollector::ProcessSensorEvents()
{	
	int result = -1;
	ASensorEvent sensorEvent;
	//Assume this is all gyro for now
	while (ASensorEventQueue_getEvents(sensorQueue,&sensorEvent, 1) > 0 )
	{
		//First time reading an event, store the timestamp and go to the next one
		if (lastTimestamp == 0)
		{
			lastTimestamp = sensorEvent.timestamp;
			rotationVector =  cv::Mat::zeros(1,3,CV_64F);
			continue;
		}
		if (resetNextTime)
		{			
			rotationVector = cv::Mat::zeros(1,3,CV_64F);
			resetNextTime = false;
		}
		
		double deltaT = (sensorEvent.timestamp - lastTimestamp) / 1000000000.0;
		double data[] = {sensorEvent.data[0],sensorEvent.data[1],sensorEvent.data[2]};
		cv::Mat	deltaVector = cv::Mat(1,3,CV_64F,data);
		deltaVector *= deltaT;
		rotationVector += deltaVector;
		lastTimestamp = sensorEvent.timestamp;
	}
}

cv::Mat SensorCollector::GetRotation()
{	
	resetNextTime = true;
	return rotationVector;
}

void SensorCollector::EnableSensors(bool enableAccel, bool enableGyro, bool enableMagneto)
{
	if (!(enableAccel || enableGyro || enableMagneto))
	{
		LOGW(LOGTAG_SENSOR,"All sensor enable selections are false...");
		return;
	}

	int defaultRate = 15;
	bool success = true;
	if (enableAccel)
	{		
		LOGI(LOGTAG_SENSOR,"Enabling accelerometer");
		int resultEnable =  ASensorEventQueue_enableSensor(sensorQueue,accelSensor);
		int resultQueue = ASensorEventQueue_setEventRate(sensorQueue,accelSensor, (1000L/ defaultRate)*1000);

		if (resultEnable < 0)
			LOGE("Error enabling accelerometer sensor");
		if (resultQueue < 0)
			LOGE("Error setting event rate for accelerometer");
		accelEnabled = (resultEnable >= 0 && resultQueue >= 0);
		success = success && accelEnabled;
	}

	if (enableGyro)
	{
		LOGI(LOGTAG_SENSOR,"Enabling gyroscope");
		int resultEnable = ASensorEventQueue_enableSensor(sensorQueue,gyroSensor);
		int resultQueue =ASensorEventQueue_setEventRate(sensorQueue,gyroSensor, (1000L/ defaultRate)*1000);

		if (resultEnable < 0)
			LOGE("Error enabling gyroscope sensor");
		if (resultQueue < 0)
			LOGE("Error setting event rate for gyroscope");

		gyroEnabled = (resultEnable >= 0 && resultQueue >= 0);
		success = success && gyroEnabled;
	}

	if (enableMagneto)
	{
		LOGI(LOGTAG_SENSOR,"Enabling magnetic field sensor");
		int resultEnable = ASensorEventQueue_enableSensor(sensorQueue,magnetoSensor);
		int resultQueue =ASensorEventQueue_setEventRate(sensorQueue,magnetoSensor, (1000L/ defaultRate)*1000);

		if (resultEnable < 0)
			LOGE("Error enabling magnetic sensor");
		if (resultQueue < 0)
			LOGE("Error setting event rate for magnetic sensor");
		magEnabled = (resultEnable >= 0 && resultQueue >= 0);
		success = success && magEnabled;
	}

	if (success)
		LOGI(LOGTAG_SENSOR,"All sensors enabled successfully!");
}

void SensorCollector::DisableSensors()
{
	bool success = true;
	int result = ASensorEventQueue_disableSensor(sensorQueue,accelSensor);
	if (result < 0)
	{
		LOGE("Error disabling accelerometer");
		success = false;
	}

	result = ASensorEventQueue_disableSensor(sensorQueue,gyroSensor);
	if (result < 0)
	{
		LOGE("Error disabling gyro sensor");
		success = false;
	}
	result = ASensorEventQueue_disableSensor(sensorQueue,magnetoSensor);
	if (result < 0)
	{	
		LOGE("Error disabling magnetic sensor");
		success = false;
	}

	if (success)
		LOGI(LOGTAG_SENSOR,"All sensors disabled successfully!");
}

