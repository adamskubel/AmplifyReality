#include "LogDefinitions.h"

#include "controllers/Controller.hpp"
#include "controllers/CalibrationController.hpp"
#include "controllers/ARController.hpp"
#include "controllers/StartupController.hpp"

#include "datacollection/ImageCollector.hpp"
#include "datacollection/ImageProcessor.hpp"
#include "display/opengl/OpenGL.hpp"

#include "positioning/QRLocator.hpp"

#include "model/FrameItem.hpp"
#include "model/Engine.hpp"
#include "model/Updateable.hpp"

#include "userinterface/AndroidInputHandler.hpp"
#include "userinterface/uimodel/Button.hpp"

#include "srutil/delegate/delegate.hpp"



#ifndef ARRUNNER_HPP_
#define ARRUNNER_HPP_

class AmplifyRunner
{	

public:
	AmplifyRunner(Engine * engine);
	~AmplifyRunner();

	void Initialize(Engine * engine);
	void DoFrame(Engine * engine);
	void Teardown(Engine * engine);

private:
	//Enumerations
	enum ActionMode
	{
		Startup = 0, QRTrack = 1, Calibrate = 2
	};

	//Members
	void ProcessFrame(Engine * engine);
	void Main_HandleTouchInput(void* sender, TouchEventArgs args);
	void Main_HandleButtonInput(void* sender, PhysicalButtonEventArgs args);
	void HandleButtonClick(void * sender, EventArgs args);
	void InitializeUserInterface(Engine * engine);
	void CheckControllerExpiry(Engine * engine);
	void ControllerExpired(CalibrationController * calibrationController);

	

	//Fields
	struct timespec lastFrameTimeStamp;	
	ActionMode currentActionMode;
	Controller * currentController;
		
};

#endif