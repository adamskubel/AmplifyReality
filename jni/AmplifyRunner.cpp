#include "AmplifyRunner.hpp"


AmplifyRunner::AmplifyRunner(Engine * engine)
{		
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

void AmplifyRunner::Teardown(Engine * engine)
{
	if (currentController != NULL)
	{	
		currentController->Teardown(engine);
		delete currentController;
		currentController = NULL;
	}
	LOGI(LOGTAG_MAIN,"AmplifyRunner Teardown Complete");
}

void AmplifyRunner::ProcessFrame(Engine* engine)
{
	struct timespec start, end;
	LOGV("Main","Frame Start");		
			
	//Make sure current controller is initialized. Where should this be?
	currentController->Initialize(engine);

	SET_TIME(&start);
	//The current controller does whatever it wants to here			
	currentController->ProcessFrame(engine);

	//Check if controller is done
	CheckControllerExpiry(engine);

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



void AmplifyRunner::CheckControllerExpiry(Engine * engine)
{
	if (currentController->IsExpired())
	{
		if (currentActionMode == Calibrate)
		{
			LOGI(LOGTAG_MAIN,"Calibration controller expired");
			//If the camera matrices were created correctly, then create a QR controller with them
			if (((CalibrationController*)currentController)->wasSuccessful())
			{				
				LOGD(LOGTAG_MAIN,"Calibration was completed successfully");
				Mat camera,distortion;
				((CalibrationController*)currentController)->getCameraMatrices(camera,distortion);
				currentController->Teardown(engine);
				delete currentController;
				currentController = new ARController(camera,distortion);	
			}
			//Otherwise, create the controller using the predefined matrix
			else
			{
				LOGD(LOGTAG_MAIN,"Calibration was not completed");
				currentController->Teardown(engine);
				delete currentController;
				currentController = new ARController();
			}		
			currentActionMode = QRTrack;
		}
		else
		{
			LOGI(LOGTAG_MAIN,"ARController expired");
			currentController->Teardown(engine);
			delete currentController;
			currentController = new CalibrationController();
			currentActionMode = Calibrate;
		}
	}
}



void AmplifyRunner::Main_HandleButtonInput(void* sender, PhysicalButtonEventArgs args)
{	
	if (args.ButtonCode == AKEYCODE_MENU)
	{
		LOGD(LOGTAG_MAIN,"Menu button pressed. Expiring current controller");
		currentController->SetExpired();
		/*ActionMode newMode;
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
		}*/
	}
}




void AmplifyRunner::InitializeUserInterface(Engine * engine)
{
	LOGD(LOGTAG_MAIN,"Initializing user interface");
	engine->inputHandler->AddGlobalButtonDelegate(ButtonEventDelegate::from_method<AmplifyRunner,&AmplifyRunner::Main_HandleButtonInput>(this));
}

void AmplifyRunner::DoFrame(Engine* engine)
{
	if (engine->app->window == NULL)
	{
		LOGV(LOGTAG_MAIN,"Window not initialized yet");
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
	if (currentController != NULL)
		delete currentController;
}