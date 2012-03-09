#include "display/opengl/OpenGL.hpp"
#include "datacollection/ImageCollector.hpp"
#include "userinterface/AndroidInputHandler.hpp"
#include "datacollection/SensorCollector.hpp"
#include <opencv2/core/core.hpp>
#include "model/network/ARCommunicator.hpp"
#include "model/PreferenceManager.hpp"
#include <jni.h>

#ifndef ENGINE_HPP_
#define ENGINE_HPP_

//Stores configuration and common objects (handle to Camera, renderer)
class Engine
{
public:
	int imageHeight;
	int imageWidth;
	struct android_app* app;
	int animating;
	
	//Important and unchanging objects
	OpenGL * glRender;
	ImageCollector * imageCollector;
	AndroidInputHandler * inputHandler;
	SensorCollector * sensorCollector;
	ARCommunicator * communicator;
	AConfiguration * androidConfiguration;
	PreferenceManager * preferenceManager;
	
	//Camera matrices
	Mat * cameraMatrix;
	Mat * distortionMatrix;

	void ExitApplication()
	{
		animating = 0;
	}


	//Retrieve current time
	void getTime(struct timespec * time)
	{
		clock_gettime(CLOCK_REALTIME,time);
	}

	cv::Size2i ImageSize()
	{
		return Size2i(imageWidth,imageHeight);
	}

	cv::Size2i ScreenSize()
	{
		return Size2i(glRender->screenWidth,glRender->screenHeight);
	}

};


#endif
