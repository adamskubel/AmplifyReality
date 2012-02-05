#include "android_native_app_glue.h"
#include "AmplifyRunner.hpp"
#include "android/sensor.h"
#include <jni.h>
#include <pthread.h>
#include <errno.h>
#include "model/network/RealmDefinition.hpp"
#include "model/network/NetworkMessages.hpp"
#include "model/network/WavefrontModel.hpp"
#include "model/network/ARCommunicator.hpp"

pthread_mutex_t incomingMutex, outgoingMutex;
static vector<IncomingMessage*> jniDataVector;
static vector<OutgoingMessage*> outgoingJNIDataVector;


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

	JNIEXPORT jobjectArray JNICALL 
		Java_com_amplifyreality_AmplifyRealityActivity_GetOutgoingMessages(JNIEnv * env, jobject  obj)
	{		
		pthread_mutex_lock(&outgoingMutex);

		if (outgoingJNIDataVector.empty())
		{
			pthread_mutex_unlock(&outgoingMutex);
			return env->NewObjectArray(1,env->FindClass("java/lang/Object"),NULL);
		}



		jclass wrapperClass = env->FindClass("com/amplifyreality/networking/message/NativeMessage");
		jmethodID initMethod = env->GetMethodID(wrapperClass,"<init>","(Ljava/lang/String;Ljava/lang/Object;)V");
		
		jobjectArray returnArray = env->NewObjectArray(outgoingJNIDataVector.size(),wrapperClass,NULL);
		

		int i= 0;
		while (!outgoingJNIDataVector.empty())
		{	
			jstring description = outgoingJNIDataVector.back()->GetDescription(env);
			jobject dataObject = outgoingJNIDataVector.back()->getJavaObject(env);

			jobject returnObject = env->NewObject(wrapperClass,initMethod,description,dataObject);
			env->SetObjectArrayElement(returnArray,i++,returnObject);

		//	delete outgoingJNIDataVector.back();
			outgoingJNIDataVector.pop_back();
		}
		
		pthread_mutex_unlock(&outgoingMutex);
		return returnArray;
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
	delete engine->communicator;

	LOGI(LOGTAG_MAIN,"Shutdown complete.");
}


void android_main(struct android_app* state)
{	
	LOG_INTRO();
	pthread_mutex_init(&incomingMutex,NULL);
	pthread_mutex_init(&outgoingMutex,NULL);



	Engine mainEngine = Engine();
	initializeEngine(state, mainEngine);

	mainEngine.communicator = new ARCommunicator();
	
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

		if ( pthread_mutex_trylock(&incomingMutex) == 0)
		{
			int count = 0;
			while (!jniDataVector.empty())
			{
				count++;
				mainEngine.communicator->AddIncomingMessage(jniDataVector.back());
				//delete jniDataVector.back();
				jniDataVector.pop_back();
			}
			if (count > 0)
				LOGD(LOGTAG_NETWORKING,"Got %d messages from JNI queue");
			pthread_mutex_unlock(&incomingMutex);
		}
		else
		{
			LOGD(LOGTAG_NETWORKING,"Unable to lock incoming mutex");
		}


		if (mainEngine.communicator->HasOutgoingMessages())
		{
			if ( pthread_mutex_trylock(&outgoingMutex) == 0)
			{
				mainEngine.communicator->GetOutgoingMessages(outgoingJNIDataVector);
				pthread_mutex_unlock(&outgoingMutex);
			}
			else
			{
				LOGD(LOGTAG_NETWORKING,"Unable to lock outgoing mutex");
			}
		}
	

		if (mainEngine.animating)
		{
			myRunner.DoFrame(&mainEngine);
		}
	}

	myRunner.~AmplifyRunner();
	shutdownEngine(&mainEngine);

}

