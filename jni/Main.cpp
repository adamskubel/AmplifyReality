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

using namespace std;
using namespace cv;

enum ActionMode
{
	QRTrack = 0, Calibrate = 1
};


ActionMode currentActionMode = QRTrack;
Controller * currentController;

struct timespec lastFrameTimeStamp;	
FrameItem * item;


void ProcessFrame(Engine* engine)
{
	struct timespec  start, end;
	LOGD("Main","Frame Start");
		
	

	//The current controller does whatever it wants to here
	currentController->ProcessFrame(engine,item);
	
	//Draw the contents of the RGBImage field, whatever they may be
	engine->glRender.render(engine->imageWidth, engine->imageHeight, item->rgbImage->ptr<uint32_t>(0));
	
	//delete item;	
}

void setActionMode(ActionMode newMode)
{
	if (currentActionMode == newMode)
		return;

	if (newMode == Calibrate)
	{
		currentController = new CalibrationController();
	} else if (currentActionMode == Calibrate)
	{
		CalibrationController* calibrationController = (CalibrationController*) (currentController);
		delete calibrationController;
	}

	currentActionMode = newMode;
}

void drawFrame(Engine* engine)
{
	if (engine->app->window == NULL)
	{
		return;
	}

	ProcessFrame(engine);

	//switch (currentActionMode)
	//{
	//case QRTrack:
	//	ProcessFrame(engine);
	//	break;
	//case Calibrate:
	//	CalibrationController* calibrationController = (CalibrationController*) (currentController);
	//	calibrationController->findCorners(engine);
	//	if (calibrationController->isExpired())
	//	{
	//		double camData[9] = {92.5, 0, 400, 0, 92.5, 240, 0, 0 ,1};
	//		Mat cameraMatrix = Mat(3,3,CV_64F,camData);
	//		qrLocator = new QRLocator(cameraMatrix);
	//		setActionMode(QRTrack);
	//	}
	//	break;
	//}

	struct timespec now;	
	SET_TIME(&now);
	LOG_TIME("Frame took %ld ms", lastFrameTimeStamp,now);
	SET_TIME(&lastFrameTimeStamp);
}

void terminateDisplay(Engine* engine)
{
	engine->animating = 0;
	engine->glRender.teardownOpenGL();
	engine->imageCollector->teardown();
	LOGI(LOGTAG_MAIN,"Teardown successful. Goodbye!");
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
			engine->glRender.initOpenGL(engine->app->window,engine->imageWidth,engine->imageHeight);
			LOGI(LOGTAG_MAIN,"OpenGL initialization complete");			
			engine->animating = 1;
		}
		break;
	case APP_CMD_TERM_WINDOW:
		terminateDisplay(engine);
		break;
	case APP_CMD_LOST_FOCUS:
		engine->animating = 0;
		break;
	}
}

static int32_t engineHandleInput(struct android_app* app, AInputEvent* event)
{
	Engine* engine = (Engine*) app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
	{
		if (((double) (AMotionEvent_getEventTime(event) / (1000000000LL))) > 0.5 && AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_UP)
		{
			switch (currentActionMode)
			{
			case QRTrack:
				engine->drawMode = (Configuration::DrawMode) ((engine->drawMode + 1) % Configuration::DrawModeSize);
				break;
			case Calibrate:
				CalibrationController* calibrationController = (CalibrationController*) (currentController);
				calibrationController->captureImage();
				break;
			}

			return 1;
		}
	} else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
	{
		if (((double) (AKeyEvent_getEventTime(event) / (1000000000LL))) > 0.5 && AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP)
		{
			LOGI(LOGTAG_MAIN,"Key event: action=%d keyCode=%d metaState=0x%x", AKeyEvent_getAction(event), AKeyEvent_getKeyCode(event), AKeyEvent_getMetaState(event));
			setActionMode((ActionMode) ((currentActionMode + 1) % 2));
			LOGI(LOGTAG_MAIN,"Mode changed");
		}
	}
	return 0;
}

void initializeEngine(Engine* engine)
{
	LOGI(LOGTAG_MAIN,"Initializing engine");
	//Mat myuv(height + height / 2, width, CV_8UC1);
	engine->imageCollector = new ImageCollector(engine->imageWidth, engine->imageHeight);
	currentController = new QRController();
}

void android_main(struct android_app* state)
{
	LOG_INTRO();

	Engine engine = Engine();

	app_dummy();

	state->userData = &engine;
	state->onAppCmd = engineHandleCommand;
	state->onInputEvent = engineHandleInput;

	item = new FrameItem();

	//Camera preview size is hardcoded for now
	engine.imageWidth = CAMERA_IMAGE_WIDTH;
	engine.imageHeight = CAMERA_IMAGE_HEIGHT;

	engine.app = state;
	engine.animating = 0;

	engine.drawMode = Configuration::GrayImage;

	initializeEngine(&engine);


	while (1)
	{

		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events, (void**) &source)) >= 0)
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
				terminateDisplay(&engine);
				return;
			}
		}

		if (engine.animating)
		{
			drawFrame(&engine);
		}
	}

	engine.glRender.teardownOpenGL();

}
