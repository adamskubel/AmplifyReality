#include "ARRunner.hpp"


ARRunner::ARRunner(Engine * engine)
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
		LocationController * qrCont = new LocationController();
		currentActionMode = QRTrack;
		LOGI(LOGTAG_MAIN,"Starting with predefined camera matrix");
	}
}

void ARRunner::Initialize(Engine * engine)
{	
	InitializeUserInterface(engine);
	
	//BG quad must be last value in update vector
	QuadBackground * quad = new QuadBackground(engine->imageWidth,engine->imageHeight);

	updateObjects.push_back(quad);
	renderObjects.push_back(quad);
}

void ARRunner::ProcessFrame(Engine* engine)
{
	struct timespec start, end;
	LOGV("Main","Frame Start");		
	
	int lastFrameItem = currentFrameItem;
	currentFrameItem = (currentFrameItem + 1) % numItems;
	FrameItem * item = items[currentFrameItem];

	item->setPreviousFrame(items[lastFrameItem]);
	item->clearOldData();
	item->drawMode = drawMode;
	
	SET_TIME(&start);

	//Make sure current controller is initialized. NEED TO MOVE THIS!	
	currentController->Initialize(engine);

	//The current controller does whatever it wants to here			
	currentController->ProcessFrame(engine,item);
	CheckControllerExpiry(engine);
	SET_TIME(&end);
	LOG_TIME("Controller",start,end);
	
	//Update
	LOGV(LOGTAG_MAIN,"Update Phase");
	for (int i=0;i<updateObjects.size();i++)
	{
		updateObjects.at(i)->Update(item);
	}
	
	//Render OpenGL Objects
	LOGV(LOGTAG_MAIN,"OpenGL Render Phase");	
	for (int i=0;i<renderObjects.size();i++)
	{
		renderObjects.at(i)->Render(engine->glRender);
	}
	
	engine->glRender->Present();

	LOGV(LOGTAG_MAIN,"Frame End");		
}



void ARRunner::CheckControllerExpiry(Engine * engine)
{
	if (currentController->isExpired())
	{
		if (currentActionMode == Calibrate)
		{
			//If the camera matrices were created correctly, then create a QR controller with them
			if (currentController->wasSuccessful())
			{
				Mat camera,distortion;
				((CalibrationController*)currentController)->getCameraMatrices(camera,distortion);
				currentController = new LocationController(camera,distortion);	
			}
			//Otherwise, create the controller using the predefined matrix
			else
			{
				currentController = new LocationController();
			}
			currentController->Initialize(engine);					
			currentActionMode = QRTrack;		
		}
	}
}

void ARRunner::Main_HandleButtonInput(void* sender, PhysicalButtonEventArgs args)
{	
	LOGI(LOGTAG_MAIN,"Received button event: %d", args.ButtonCode);
	if (args.ButtonCode == AKEYCODE_MENU)
	{
		ActionMode newMode;
		switch (currentActionMode)
		{
		case (QRTrack):
			currentActionMode = Calibrate;
			delete currentController;
			currentController = new CalibrationController();
			break;
		case (Calibrate):
			currentActionMode = QRTrack;
			delete currentController;
			currentController = new LocationController();
			break;
		}
	}
}

void ARRunner::Main_HandleTouchInput(void* sender, TouchEventArgs args)
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



void ARRunner::InitializeUserInterface(Engine * engine)
{
	LOGD(LOGTAG_MAIN,"Initializing user interface");
	engine->inputHandler->AddGlobalButtonDelegate(ButtonEventDelegate::from_method<ARRunner,&ARRunner::Main_HandleButtonInput>(this));
	engine->inputHandler->AddGlobalTouchDelegate(TouchEventDelegate::from_method<ARRunner,&ARRunner::Main_HandleTouchInput>(this));

}

void ARRunner::DoFrame(Engine* engine)
{
	if (engine->app->window == NULL)
	{
		return;
	}

	ProcessFrame(engine);

	struct timespec now;	
	SET_TIME(&now);
	LOG_TIME("Frame", lastFrameTimeStamp,now);
	SET_TIME(&lastFrameTimeStamp);
}



ARRunner::~ARRunner()
{
	while (!updateObjects.empty())
	{
		delete updateObjects.back();
		updateObjects.pop_back();
	}

	while(!renderObjects.empty())
	{
		delete renderObjects.back();
		renderObjects.pop_back();
	}
}