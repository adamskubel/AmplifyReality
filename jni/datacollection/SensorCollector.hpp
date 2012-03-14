#include "android/sensor.h"
#include "android_native_app_glue.h"
#include <vector>
#include "LogDefinitions.h"

#ifndef SENSORCOLLECTOR_HPP_
#define SENSORCOLLECTOR_HPP_

class SensorCollector
{
public:
	SensorCollector(ASensorManager* sensorManagerr, ALooper * looper);
	~SensorCollector();
	void EnableSensors(bool enableAccel, bool enableGyro, bool enableMagneto);
	void DisableSensors();
	//void ReadSensors(std::vector<ASensorEvent*> * sensorEvents);
	void ProcessSensorEvents();
	cv::Mat GetRotation();
	void ClearRotation();

private:
	int64_t lastTimestamp;
	cv::Mat rotationVector;
	bool accelEnabled, gyroEnabled, magEnabled;
	bool resetNextTime;
	const ASensor * accelSensor, *gyroSensor, *magnetoSensor;
	ASensorEventQueue *sensorQueue ;//, *accelQueue,*gyroQueue,*magnetoQueue;
};

#endif