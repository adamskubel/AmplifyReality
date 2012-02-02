#include "android_native_app_glue.h"
#include "AmplifyRunner.hpp"
#include "android/sensor.h"
#include <jni.h>
#include <pthread.h>
#include <errno.h>


pthread_mutex_t myMutex;

static vector<std::string*> jniDataVector;


extern "C"
{
	JNIEXPORT void JNICALL 
		Java_com_amplifyreality_AmplifyRealityActivity_OnMessage(JNIEnv * env, jobject  obj, jstring javaString)
	{
		pthread_mutex_lock(&myMutex);
		LOGD(LOGTAG_NETWORKING,"JNI thread locked mutex");

		std::string * cString = new std::string(env->GetStringUTFChars(javaString,JNI_FALSE));
		jniDataVector.push_back(cString);

		pthread_mutex_unlock(&myMutex);
		LOGD(LOGTAG_NETWORKING,"JNI thread unlocked mutex");
	}
}



static void engineHandleCommand(struct android_app* app, int32_t cmd);
static int32_t engineHandleInput(struct android_app* app, AInputEvent* inputEvent);

void initializeEngine(struct android_app* state, Engine & engine);
void shutdownEngine(Engine* engine);


struct ARUserData
{	
	AmplifyRunner * runner;
	Engine * engine;
};

void engineHandleCommand(struct android_app* app, int32_t cmd)
{
	ARUserData * data = (ARUserData*) app->userData;
	Engine* engine = data->engine;

	switch (cmd)
	{
	case APP_CMD_INIT_WINDOW:
		if (data->engine->app->window != NULL)
		{
			LOGI(LOGTAG_MAIN,"OS Has Initialized Window");

			engine->glRender = new OpenGL(engine->app->window);	
			data->runner->Initialize(engine);
			engine->animating = 1;
		}
		break;
	case APP_CMD_TERM_WINDOW:
		shutdownEngine(engine);
		break;
	case APP_CMD_GAINED_FOCUS:
		//Enable all sensors, start animation
	/*	if (engine->sensorCollector != NULL)
		{
			engine->sensorCollector->EnableSensors(false,true,false);
		}*/
		engine->animating = 1;
		break;
	case APP_CMD_LOST_FOCUS:
		//Disable all sensors and stop animation
		if (engine->sensorCollector != NULL)
		{
			engine->sensorCollector->DisableSensors();
		}
		engine->animating = 0;
		break;
    }
}

int32_t engineHandleInput(struct android_app* app, AInputEvent* inputEvent)
{
	Engine* engine = (Engine*) ((ARUserData*) app->userData)->engine;
	return engine->inputHandler->HandleInputEvent(app,inputEvent);
}


void initializeEngine(struct android_app* state, Engine & engine)
{
	LOGI(LOGTAG_MAIN,"Initializing engine");

	//Define engine properties
	engine.animating = 0;
			
	//Call this to ensure "glue isn't stripped" (w/e that means..)
	app_dummy();

	state->onAppCmd = engineHandleCommand;
	state->onInputEvent = engineHandleInput;

	//Store state in engine
	engine.app = state;
	
	//Camera preview size is hardcoded for now
	//TODO: Read from VC
	engine.imageWidth = CAMERA_IMAGE_WIDTH;
	engine.imageHeight = CAMERA_IMAGE_HEIGHT;

	//Initialize objects
	try
	{
		engine.imageCollector = new ImageCollector(engine.imageWidth, engine.imageHeight);
	}
	catch (Exception & e)
	{
		engine.imageCollector = NULL;
	}
	engine.inputHandler = new AndroidInputHandler();
	engine.sensorCollector = new SensorCollector(ASensorManager_getInstance(), state->looper);
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
		delete engine->glRender;
		LOGI(LOGTAG_MAIN,"OpenGL Shutdown");
	}
	
	LOGI(LOGTAG_MAIN,"Deleting engine objects");
	delete engine->imageCollector; 
	delete engine->inputHandler;
	delete engine->sensorCollector;
	delete engine->messageQueue;

	LOGI(LOGTAG_MAIN,"Shutdown complete.");
}


void android_main(struct android_app* state)
{	
	LOG_INTRO();
	pthread_mutex_init(&myMutex,NULL);



	Engine mainEngine = Engine();
	initializeEngine(state, mainEngine);

	mainEngine.messageQueue = new vector<string*>();
	
	AmplifyRunner myRunner = AmplifyRunner(&mainEngine);

	struct ARUserData myData;
	memset(&myData,0,sizeof(ARUserData));

	myData.engine = &mainEngine;
	myData.runner = &myRunner;


	state->userData = &myData;

	

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
			
			//Process sensor events
			if (ident == LOOPER_ID_USER)
			{
				mainEngine.sensorCollector->ProcessSensorEvents();
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0)
			{
				LOGI(LOGTAG_MAIN,"Engine thread destroy requested!");
				shutdownEngine(&mainEngine);
				return;
			}
		}


		//Check for messages in JNI queue

		if ( pthread_mutex_trylock(&myMutex) == 0)
		{
			if (!jniDataVector.empty())
			{
				std::string * safeString = new std::string(jniDataVector.back()->c_str());

				LOGD(LOGTAG_NETWORKING,"Network message: %s",safeString->c_str());
				
				delete jniDataVector.back();
				jniDataVector.pop_back();

				mainEngine.messageQueue->push_back(safeString);
			}
			pthread_mutex_unlock(&myMutex);
		}
		else
		{
			LOGI(LOGTAG_NETWORKING,"Unable to lock mutex");
		}

		if (mainEngine.animating)
		{
			myRunner.DoFrame(&mainEngine);
		}
	}

	myRunner.~AmplifyRunner();
	shutdownEngine(&mainEngine);

}

