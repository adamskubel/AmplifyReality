#include "positioning/QRFinder.hpp"
#include "LogDefinitions.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "android_native_app_glue.h"
#include "display/NativeWindowRenderer.hpp"

#include "controllers/Controller.hpp"
#include "controllers/CalibrationController.hpp"
#include "controllers/QRController.hpp"

#include "datacollection/ImageCollector.hpp"
#include "datacollection/ImageProcessor.hpp"
#include "display/opengl/OpenGLRenderer.hpp"
#include "positioning/QRLocator.hpp"
#include "model/FrameItem.hpp"
#include "model/Engine.hpp"

#include "userinterface/AndroidInputHandler.hpp"

using namespace std;
using namespace cv;

enum ActionMode
{
	QRTrack = 0, Calibrate = 1
};

ActionMode currentActionMode = QRTrack;
AndroidInputHandler inputHandler;
Controller * currentController;
Engine mainEngine;

//locals
struct timespec lastFrameTimeStamp;	
FrameItem ** items;
const int numItems = 2;
int currentFrameItem = 0;

void ProcessFrame(Engine* engine)
{
	struct timespec  start, end;
	LOGV("Main","Frame Start");		
	
	int lastFrameItem = currentFrameItem;
	currentFrameItem = (currentFrameItem + 1) % numItems;
	FrameItem * item = items[currentFrameItem];
	item->clearOldData();

	SET_TIME(&start);
	//The current controller does whatever it wants to here
	currentController->ProcessFrame(engine,item);
	SET_TIME(&end);
	LOG_TIME("Controller",start,end);
	
	//Draw the contents of the RGBImage field, whatever they may be
	engine->glRender->render(engine->imageWidth, engine->imageHeight, item->rgbImage->ptr<uint32_t>(0));
	LOGV("Main","Frame End");		
}

void Main_HandleButtonInput(void* sender, PhysicalButtonEventArgs args)
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
			currentController = new QRController();
			break;
		}
	}
}

void Main_HandleTouchInput(void* sender, TouchEventArgs args)
{
	switch (currentActionMode)
	{
	case (Calibrate):
		((CalibrationController*)currentController)->captureImage();
		break;
	default:
		switch (mainEngine.drawMode)
		{
		case(Configuration::BinaryImage):
			mainEngine.drawMode = Configuration::ColorImage;
			break;
		case(Configuration::GrayImage):
			mainEngine.drawMode = Configuration::BinaryImage;
			break;
		case(Configuration::ColorImage):
			mainEngine.drawMode = Configuration::GrayImage;
			break;
		}
		break;
	}
}



void initializeUserInterface()
{
	inputHandler.AddGlobalButtonListener(Main_HandleButtonInput);
	inputHandler.AddGlobalTouchListener(Main_HandleTouchInput);
}



void shutdownEngine(Engine* engine)
{
	LOGI(LOGTAG_MAIN,"Shutting down");
	engine->animating = 0;
	
	if (engine->imageCollector != NULL)
	{
		engine->imageCollector->teardown();
		LOGI(LOGTAG_MAIN,"Image Collector Shutdown");
	}

	if (engine->glRender != NULL)
	{	
		engine->glRender->teardownOpenGL();
		LOGI(LOGTAG_MAIN,"OpenGL Shutdown");
	}
	
	LOGI(LOGTAG_MAIN,"Shutdown complete.");
}

void drawFrame(Engine* engine)
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

static void engineHandleCommand(struct android_app* app, int32_t cmd)
{
	Engine* engine = (Engine*) app->userData;
	switch (cmd)
	{
	case APP_CMD_INIT_WINDOW:
		if (engine->app->window != NULL)
		{
			LOGI(LOGTAG_MAIN,"Initializing OpenGL");
			engine->glRender->initOpenGL(engine->app->window,engine->imageWidth,engine->imageHeight);
			LOGI(LOGTAG_MAIN,"OpenGL initialization complete");			
			engine->animating = 1;
		}
		break;
	case APP_CMD_TERM_WINDOW:
		shutdownEngine(engine);
		break;
	case APP_CMD_LOST_FOCUS:
		engine->animating = 0;
		break; 
	}
}

static int32_t engineHandleInput(struct android_app* app, AInputEvent* inputEvent)
{
	return inputHandler.HandleInputEvent(app,inputEvent);
}

void initializeEngine(struct android_app* state, Engine & engine)
{
	LOGI(LOGTAG_MAIN,"Initializing engine");

	//Define engine properties
	engine.animating = 0;
	
	//Camera preview size is hardcoded for now
	//TODO: Read from VC
	engine.imageWidth = HTC_SENSATION_CAMERA_IMAGE_WIDTH;
	engine.imageHeight = HTC_SENSATION_CAMERA_IMAGE_HEIGHT;

	//Start in gray - configurable?
	engine.drawMode = Configuration::GrayImage;
		
	//Call this for some reason. WHY??
	app_dummy();

	state->userData = &engine;
	state->onAppCmd = engineHandleCommand;
	state->onInputEvent = engineHandleInput;
	//Store state in engine
	engine.app = state;

	items = new FrameItem*[numItems];
	for (int i=0;i < numItems; i ++)
	{
		items[i] = new FrameItem();
	}
			
	//Initialize objects
	engine.glRender = new OpenGLRenderer();
	engine.imageCollector = new ImageCollector(engine.imageWidth, engine.imageHeight);
	currentController = new QRController();

}

void android_main(struct android_app* state)
{	
	LOG_INTRO();
	
	mainEngine = Engine();
	inputHandler = AndroidInputHandler();

	initializeEngine(state, mainEngine);
	initializeUserInterface();
	
	while (1)
	{
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue to draw the next frame of animation.
		while ((ident = ALooper_pollAll(mainEngine.animating ? 0 : -1, NULL, &events, (void**) &source)) >= 0)
		{
			// Process this event.
			if (source != NULL)
			{
				source->process(state, source);
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0)
			{
				LOGI("Main","Engine thread destroy requested!");
				shutdownEngine(&mainEngine);
				return;
			}
		}
		if (mainEngine.animating)
		{
			drawFrame(&mainEngine);
		}
	}
	shutdownEngine(&mainEngine);

}

