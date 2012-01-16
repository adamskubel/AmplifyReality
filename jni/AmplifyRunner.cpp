#include "AmplifyRunner.hpp"


AmplifyRunner::AmplifyRunner(Engine * engine)
{
	//Start in gray - configurable?
	drawMode = Configuration::GrayImage;	
	currentFrameItem = 0;
	
	items = new FrameItem*[numItems];
	for (int i=0;i < numItems; i ++)
	{
		items[i] = new FrameItem();
	}
	LOGI(LOGTAG_MAIN,"Created %d frame items",numItems);
		
	if (USE_CALCULATED_CAMERA_MATRIX)
	{
		currentController = new CalibrationController();
		currentActionMode = Calibrate;
		LOGI(LOGTAG_MAIN,"Starting in calibration mode");
	}
	else
	{
		currentController = new ARController();
		currentActionMode = QRTrack;
		LOGI(LOGTAG_MAIN,"Starting with predefined camera matrix");
	}
}

void AmplifyRunner::Initialize(Engine * engine)
{	
	InitializeUserInterface(engine);	
}

void AmplifyRunner::ProcessFrame(Engine* engine)
{
	struct timespec start, end;
	LOGV("Main","Frame Start");		
	
	int lastFrameItem = currentFrameItem;
	currentFrameItem = (currentFrameItem + 1) % numItems;
	LOGV(LOGTAG_MAIN,"Using item %d",currentFrameItem);

	FrameItem * item = items[currentFrameItem];

	item->setPreviousFrame(items[lastFrameItem]);
	
	LOGV(LOGTAG_MAIN,"Clearing old data from item");
	item->clearOldData();
	item->drawMode = drawMode;
	
	//Make sure current controller is initialized. Where should this be?
	currentController->Initialize(engine);

	SET_TIME(&start);
	//The current controller does whatever it wants to here			
	currentController->ProcessFrame(engine,item);

	//Check if controller is done
	CheckControllerExpiry();

	SET_TIME(&end);
	LOG_TIME("Controller",start,end);
	
	//Update
	LOGV(LOGTAG_MAIN,"Update Phase");
		
	//OpenGL Rendering 
	LOGV(LOGTAG_MAIN,"Render Phase");	
	engine->glRender->StartFrame();
	currentController->Render(engine->glRender);
	engine->glRender->EndFrame();

	LOGV(LOGTAG_MAIN,"Frame End");		
}



void AmplifyRunner::CheckControllerExpiry()
{
	if (currentController->isExpired())
	{
		if (currentActionMode == Calibrate)
		{
			LOGI(LOGTAG_MAIN,"Calibration controller expired");
			//If the camera matrices were created correctly, then create a QR controller with them
			if (currentController->wasSuccessful())
			{
				Mat camera,distortion;
				((CalibrationController*)currentController)->getCameraMatrices(camera,distortion);
				delete currentController;
				currentController = new ARController(camera,distortion);	
			}
			//Otherwise, create the controller using the predefined matrix
			else
			{
				delete currentController;
				currentController = new ARController();
			}		
			currentActionMode = QRTrack;
		}
	}
}



void AmplifyRunner::Main_HandleButtonInput(void* sender, PhysicalButtonEventArgs args)
{	
	LOGD(LOGTAG_MAIN,"Received button event: %d", args.ButtonCode);
	if (args.ButtonCode == AKEYCODE_MENU)
	{
		ActionMode newMode;
		switch (currentActionMode)
		{
		case (QRTrack):
			delete currentController;
			LOGD(LOGTAG_MAIN,"Deleted ARController from Main");
			currentController = new CalibrationController();
			currentActionMode = Calibrate;
			break;
		case (Calibrate):
			currentActionMode = QRTrack;
			delete currentController;	
			LOGD(LOGTAG_MAIN,"Creating new ARController");
			currentController = new ARController();	
			break;
		}
	}
}

void AmplifyRunner::Main_HandleTouchInput(void* sender, TouchEventArgs args)
{
	LOGI(LOGTAG_MAIN,"Received touch event: %d", args.InputType);
	switch (drawMode)
	{
	case(Configuration::BinaryImage):
		drawMode = Configuration::ColorImage;
		break;
	case(Configuration::GrayImage):
		drawMode = Configuration::BinaryImage;
		break;
	case(Configuration::ColorImage):
		drawMode = Configuration::GrayImage;
		break;
	}
}



void AmplifyRunner::InitializeUserInterface(Engine * engine)
{
	LOGD(LOGTAG_MAIN,"Initializing user interface");
	engine->inputHandler->AddGlobalButtonDelegate(ButtonEventDelegate::from_method<AmplifyRunner,&AmplifyRunner::Main_HandleButtonInput>(this));
	engine->inputHandler->AddGlobalTouchDelegate(TouchEventDelegate::from_method<AmplifyRunner,&AmplifyRunner::Main_HandleTouchInput>(this));

}

void AmplifyRunner::DoFrame(Engine* engine)
{
	if (engine->app->window == NULL)
	{
		return;
	}

	ProcessFrame(engine);

	struct timespec now;	
	SET_TIME(&now);
	LOG_TIME_DEBUG("Frame", lastFrameTimeStamp,now);
	SET_TIME(&lastFrameTimeStamp);
}



AmplifyRunner::~AmplifyRunner()
{
	delete currentController;
}