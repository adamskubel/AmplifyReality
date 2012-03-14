#include "AmplifyRunner.hpp"


AmplifyRunner::AmplifyRunner(Engine * engine)
{		
	currentController = new StartupController();
	LOGI(LOGTAG_MAIN,"Starting controller");
}
//
//void AmplifyRunner::Initialize(Engine * engine)
//{	
//	LOGI(LOGTAG_MAIN,
//		"Initalizing application. Engine = [screenWidth = %d, screenHeight = %d, imageWidth = %d, imageHeight = %d]",
//		engine->glRender->screenWidth,engine->glRender->screenHeight,engine->imageWidth,engine->imageHeight);
//	
//	InitializeUserInterface(engine);	
//}

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
	LOGV("Main","Frame Start");		
			
	//Make sure current controller is initialized.
	currentController->Initialize(engine);

	//The current controller does whatever it wants to here			
	currentController->ProcessFrame(engine);

	//Check if controller is done
	CheckControllerExpiry(engine);

		
	//OpenGL Rendering 
	engine->glRender->StartFrame();
	currentController->Render(engine->glRender);
	engine->glRender->EndFrame();

	LOGV(LOGTAG_MAIN,"Frame End");		
}



void AmplifyRunner::CheckControllerExpiry(Engine * engine)
{
	if (currentController->IsExpired())
	{
		LOGI(LOGTAG_MAIN,"Switching controller");
		Controller * tmp = currentController->GetSuccessor(engine);
		delete currentController;
		currentController = tmp;
	}
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