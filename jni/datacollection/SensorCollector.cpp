#include "datacollection/SensorCollector.hpp"

static cv::Mat getRotationMatrixFromVector(float rotationVector[4]) 
{

	//float q0;
	float q1 = rotationVector[0];
	float q2 = rotationVector[1];
	float q3 = rotationVector[2];
	float q0 = rotationVector[3];
	

	float sq_q1 = 2 * q1 * q1;
	float sq_q2 = 2 * q2 * q2;
	float sq_q3 = 2 * q3 * q3;
	float q1_q2 = 2 * q1 * q2;
	float q3_q0 = 2 * q3 * q0;
	float q1_q3 = 2 * q1 * q3;
	float q2_q0 = 2 * q2 * q0;
	float q2_q3 = 2 * q2 * q3;
	float q1_q0 = 2 * q1 * q0;

	float R[16];
	R[0] = 1 - sq_q2 - sq_q3;
	R[1] = q1_q2 - q3_q0;
	R[2] = q1_q3 + q2_q0;
	R[3] = 0.0f;

	R[4] = q1_q2 + q3_q0;
	R[5] = 1 - sq_q1 - sq_q3;
	R[6] = q2_q3 - q1_q0;
	R[7] = 0.0f;

	R[8] = q1_q3 - q2_q0;
	R[9] = q2_q3 + q1_q0;
	R[10] = 1 - sq_q1 - sq_q2;
	R[11] = 0.0f;

	R[12] = R[13] = R[14] = 0.0f;
	R[15] = 1.0f;

	return cv::Mat(4,4,CV_32F,R);
}

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
	rotationVector =  cv::Mat::eye(4,4,CV_32F);
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
		LOGV(LOGTAG_SENSOR,"Processing sensor event");
		//First time reading an event, store the timestamp and go to the next one
		if (lastTimestamp == 0)
		{
			lastTimestamp = sensorEvent.timestamp;
			rotationVector =  cv::Mat::eye(4,4,CV_32F);
			continue;
		}
		if (resetNextTime)
		{			
			rotationVector = cv::Mat::eye(4,4,CV_32F);
			resetNextTime = false;
		}
		
		double deltaT = (sensorEvent.timestamp - lastTimestamp) / 1000000000.0;
		float axisX = -sensorEvent.data[1];
		float axisY = -sensorEvent.data[0];
		float axisZ = -sensorEvent.data[2];

		// Calculate the angular speed of the sample
		float omegaMagnitude = sqrt(axisX*axisX + axisY*axisY + axisZ*axisZ);

		LOGD(LOGTAG_SENSOR,"OmegaMagnitue=%f",omegaMagnitude);
		// Normalize the rotation vector if it's big enough to get the axis
		if (omegaMagnitude > .001f) 
		{
			axisX /= omegaMagnitude;
			axisY /= omegaMagnitude;
			axisZ /= omegaMagnitude;
		}

		// Integrate around this axis with the angular speed by the timestep
		// in order to get a delta rotation from this sample over the timestep
		// We will convert this axis-angle representation of the delta rotation
		// into a quaternion before turning it into the rotation matrix.
		float thetaOverTwo = omegaMagnitude * deltaT / 2.0f;
		float sinThetaOverTwo = sin(thetaOverTwo);
		float cosThetaOverTwo = cos(thetaOverTwo);
		
		float deltaRotationVector[] = {0,0,0,0};
		deltaRotationVector[0] = sinThetaOverTwo * axisX;
		deltaRotationVector[1] = sinThetaOverTwo * axisY;
		deltaRotationVector[2] = sinThetaOverTwo * axisZ;
		deltaRotationVector[3] = cosThetaOverTwo;
		
		
	/*	double deltaT = (sensorEvent.timestamp - lastTimestamp) / 1000000000.0;
		double data[] = {sensorEvent.data[0],sensorEvent.data[1],sensorEvent.data[2]};
		cv::Mat	deltaVector = cv::Mat(3,1,CV_64F,data);
		deltaVector *= deltaT;
		rotationVector += deltaVector;*/
		lastTimestamp = sensorEvent.timestamp;
		LOGD(LOGTAG_SENSOR,"Creating rotation matrix from vector (%f,%f,%f,%f)",deltaRotationVector[0],deltaRotationVector[1],deltaRotationVector[2],deltaRotationVector[3]);
		cv::Mat deltaMat = getRotationMatrixFromVector(deltaRotationVector);
		rotationVector *= deltaMat;
	}
}

cv::Mat SensorCollector::GetRotation()
{	
	//cv::Mat doubleMat(4,4,CV_32F);
	//rotationVector.convertTo(doubleMat,CV_32F);
	//resetNextTime = true;
	return rotationVector;
}

void SensorCollector::ClearRotation()
{
	resetNextTime = true;
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

