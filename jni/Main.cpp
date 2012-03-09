#include "android_native_app_glue.h"
#include "AmplifyRunner.hpp"
#include "android/sensor.h"
#include <jni.h>
#include <pthread.h>
#include <errno.h>
#include "model/network/NetworkMessages.hpp"
#include "model/network/ARCommunicator.hpp"
#include "util/JNIUtils.hpp"

pthread_mutex_t incomingMutex;
static vector<IncomingMessage*> jniDataVector;
static volatile bool envInitialized;
static jobject arClientObject, myActivity;
static JavaVM * javaVM;

map<std::string, jclass> JNIUtils::javaClassMap;

extern "C"
{
	JNIEXPORT void JNICALL 
		Java_com_amplifyreality_AmplifyRealityActivity_OnMessage(JNIEnv * env, jobject  obj, jstring javaString, jobject dataObject)
	{
		pthread_mutex_lock(&incomingMutex);
		LOGD(LOGTAG_NETWORKING,"JNI thread locked mutex");

		std::string * cString = new std::string(env->GetStringUTFChars(javaString,JNI_FALSE));		

		if (cString->compare("RealmObject") == 0)
		{
			LOGD(LOGTAG_JNI,"Creating new realm object");
			RealmDefinition * realm = RealmDefinition::FromJNIEnv(env,dataObject);
			jniDataVector.push_back(realm);
		}
		else if(cString->compare("WavefrontObject") == 0)
		{			
			LOGD(LOGTAG_JNI,"Received wavefront object");
			jniDataVector.push_back(WavefrontModel::FromJNI(env,dataObject));
		}		
		else
		{	
			LOGD(LOGTAG_JNI,"String message added.");
			jniDataVector.push_back(new StringMessage(cString));
		}
		
		pthread_mutex_unlock(&incomingMutex);
		LOGD(LOGTAG_NETWORKING,"JNI thread unlocked mutex");
	}
	
	jint JNI_OnLoad(JavaVM* vm, void* reserved)
	{		
		javaVM = vm;
		LOGI(LOGTAG_JNI,"JNI_OnLoad called");
		return JNI_VERSION_1_6;
	}

	JNIEXPORT void JNICALL 
		Java_com_amplifyreality_AmplifyRealityActivity_SetClientObject(JNIEnv * env, jobject  obj, jobject _myActivity, jobject _arClientObject)
	{		
		myActivity = _myActivity;
		arClientObject = _arClientObject;
	
		LOGI(LOGTAG_JNI,"Looking up classes");
		JNIUtils::AddClass(env,"com/amplifyreality/networking/message/NativeMessage");
		JNIUtils::AddClass(env,"com/amplifyreality/networking/model/Vector3");
		JNIUtils::AddClass(env,"com/amplifyreality/networking/model/ARObject");
		
		envInitialized = true;
		LOGI(LOGTAG_JNI,"Java environment initialized.");
	}
}


static void engineHandleCommand(struct android_app* app, int32_t cmd);
static void sendMessagesToJNI(vector<OutgoingMessage*> messages);
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
	LOGD(LOGTAG_INPUT,"Handling OS cmd: %d",cmd);

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
		engine->animating = 1;
		break;
	case APP_CMD_LOST_FOCUS:
		engine->animating = 0;
		//shutdownEngine(engine);
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
	engine.animating = 1;
				
	//Call this to ensure "glue isn't stripped" (w/e that means..)
	app_dummy();

	state->onAppCmd = engineHandleCommand;
	state->onInputEvent = engineHandleInput;

	engine.app = state;
	
	//Default image size
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

	LOGI(LOGTAG_MAIN,"Initialize stateful classes");
	engine.inputHandler = new AndroidInputHandler();
	engine.sensorCollector = new SensorCollector(ASensorManager_getInstance(), state->looper);	
	engine.communicator = new ARCommunicator(arClientObject);	
	engine.preferenceManager = new PreferenceManager(myActivity);

}


void shutdownEngine(Engine* engine)
{
	LOGI(LOGTAG_MAIN,"Shutting down");
	engine->animating = 0;
	
	/*if (engine->sensorCollector != NULL)
	{
		engine->sensorCollector->disablesensors();
	}*/
	
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
	delete engine->communicator;

	LOGI(LOGTAG_MAIN,"Shutdown complete.");
}

void android_main(struct android_app* state)
{	
	LOG_INTRO();
	if (!envInitialized)
	{
		LOGE("ERROR! JAVA DID NOT INITIALIZE FIRST. Exiting!");
		return;
	}

	pthread_mutex_init(&incomingMutex,NULL);
	jniDataVector.clear();	

	Engine mainEngine = Engine();
	initializeEngine(state, mainEngine);

	
	AmplifyRunner myRunner = AmplifyRunner(&mainEngine);

	struct ARUserData myData;
	memset(&myData,0,sizeof(ARUserData));

	myData.engine = &mainEngine;
	myData.runner = &myRunner;


	state->userData = &myData;

	bool running = true;
	while (running)
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
		
		if (mainEngine.animating == 1)
		{	
			mainEngine.communicator->Update(javaVM);

			//Check for messages in JNI queue
			if ( pthread_mutex_trylock(&incomingMutex) == 0)
			{
				int count = 0;
				while (!jniDataVector.empty())
				{
					count++;
					mainEngine.communicator->AddIncomingMessage(jniDataVector.back());
					jniDataVector.pop_back();
				}
				if (count > 0)
					LOGD(LOGTAG_NETWORKING,"Got %d messages from JNI queue",count);
				pthread_mutex_unlock(&incomingMutex);
			}
			else
			{
				LOGD(LOGTAG_NETWORKING,"Unable to lock incoming mutex");
			}

			try
			{
				myRunner.DoFrame(&mainEngine);
			}
			catch (exception & e)
			{
				LOGE("Exiting due to caught exception. Message=%s",e.what());
				myRunner.Teardown(&mainEngine);
				shutdownEngine(&mainEngine);
			}
		}

		//Check if state has changed during animation loop
		if (mainEngine.animating == 0)
		{
			LOGW(LOGTAG_MAIN,"Exiting due to internal user command.");
			ANativeActivity_finish(state->activity);
		}
	}
	myRunner.~AmplifyRunner();
	shutdownEngine(&mainEngine);
	//ANativeActivity_finish(state->activity);
}

