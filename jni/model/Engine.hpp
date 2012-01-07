#include "display/opengl/OpenGL.hpp"
#include "datacollection/ImageCollector.hpp"
#include "userinterface/AndroidInputHandler.hpp"

#ifndef ENGINE_HPP_
#define ENGINE_HPP_

//Stores configuration and common objects (handle to Camera, renderer)
class Engine
{
public:
	int imageHeight;
	int imageWidth;
	int screenWidth;
	int screenHeight;
	struct android_app* app;
	int animating;
	
	//Important and unchanging objects
	OpenGL * glRender;
	ImageCollector * imageCollector;
	AndroidInputHandler * inputHandler;



	////Putting some configuration stuff in here for now
	//Configuration::DrawMode drawMode;

	//Camera matrices
	Mat * cameraMatrix;
	Mat * distortionMatrix;
};


#endif
