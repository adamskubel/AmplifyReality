#include "LogDefinitions.h"

#include "controllers/Controller.hpp"
#include "controllers/CalibrationController.hpp"
#include "controllers/LocationController.hpp"

#include "datacollection/ImageCollector.hpp"
#include "datacollection/ImageProcessor.hpp"
#include "display/opengl/OpenGL.hpp"
#include "display/opengl/QuadBackground.hpp"

#include "positioning/QRLocator.hpp"

#include "model/FrameItem.hpp"
#include "model/Engine.hpp"
#include "model/Updateable.hpp"

#include "userinterface/AndroidInputHandler.hpp"
#include "userinterface/uimodel/Button.hpp"

#include "srutil/delegate/delegate.hpp"


#ifndef ARRUNNER_HPP_
#define ARRUNNER_HPP_

class ARRunner
{	

public:
	ARRunner(Engine * engine);
	~ARRunner();

	void Initialize(Engine * engine);
	void DoFrame(Engine * engine);
	void Cleanup();

private:
	//Enumerations
	enum ActionMode
	{
		QRTrack = 0, Calibrate = 1
	};

	//Members
	void ProcessFrame(Engine * engine);
	void Main_HandleTouchInput(void* sender, TouchEventArgs args);
	void Main_HandleButtonInput(void* sender, PhysicalButtonEventArgs args);
	void HandleButtonClick(void * sender, EventArgs args);
	void InitializeUserInterface(Engine * engine);
	void CheckControllerExpiry();
	void ControllerExpired(CalibrationController * calibrationController);

	//Constants
	static const int numItems = 2; //Number of previous frames to store

	//Fields
	struct timespec lastFrameTimeStamp;	
	FrameItem ** items;
	int currentFrameItem;
	ActionMode currentActionMode;
	Configuration::DrawMode drawMode;
	Controller * currentController;
	QuadBackground * quadBackground;

	/*std::vector<Updateable*> updateObjects;
	std::vector<OpenGLRenderable*> renderObjects;*/

	
};

#endif