#include "display/opengl/OpenGLRenderer.hpp"
#include "datacollection/ImageCollector.hpp"

#ifndef ENGINE_HPP_
#define ENGINE_HPP_

namespace Configuration
{
	enum DrawMode
	{
		ColorImage = 0, GrayImage = 1, BinaryImage = 2
	};
	static const int DrawModeSize = 3;
}

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
	OpenGLRenderer * glRender;
	ImageCollector * imageCollector;



	//Putting some configuration stuff in here for now
	Configuration::DrawMode drawMode;

	//Camera matrices
	Mat * cameraMatrix;
	Mat * distortionMatrix;
};


#endif
