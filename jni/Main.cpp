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
#include "model/concurrent/ThreadedClass.hpp"

pthread_mutex_t incomingMutex, outgoingMutex;
static vector<IncomingMessage*> jniDataVector;
static vector<OutgoingMessage*> outgoingJNIDataVector;
static std::string connectionString;
static bool connectedToServer;
static bool softKeyboardOpen;


//static JNIEnv * javaEnvironment;
//static JavaVM * javaVM;
//static jobject clientObject;

//static volatile bool javaEnvInitialized = false;


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

	

	JNIEXPORT void JNICALL 
		Java_com_amplifyreality_AmplifyRealityActivity_SetConnected(JNIEnv * env, jobject  obj, jboolean connected)
	{
		pthread_mutex_lock(&incomingMutex);
		LOGD(LOGTAG_JNI,"Setting connected state to %d",connected);
		connectedToServer =  (connected != 0);
		pthread_mutex_unlock(&incomingMutex);
	}

	//GetConnectionString()
	JNIEXPORT jstring JNICALL 
		Java_com_amplifyreality_AmplifyRealityActivity_GetConnectionString(JNIEnv * env, jobject  obj)
	{
		pthread_mutex_lock(&incomingMutex);
		LOGD(LOGTAG_JNI,"Returning connection string:%s",connectionString.c_str());
		pthread_mutex_unlock(&incomingMutex);
		return env->NewStringUTF(connectionString.c_str());
	}


	//SetKeyboardState(bool)
	JNIEXPORT void JNICALL Java_com_amplifyreality_AmplifyRealityActivity_SetKeyboardState(JNIEnv * env, jobject  obj, jboolean keyboardOpen)
	{
		softKeyboardOpen = (keyboardOpen != 0);
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

	//LOGV(LOGTAG_INPUT,"Main: ContentRect[%d,%d,%d,%d]",app->contentRect.left,app->contentRect.top,app->contentRect.right,app->contentRect.bottom);

		//AConfiguration_setKeysHidden(app->config,ACONFIGURATION_KEYSHIDDEN_NO);
	//engine->androidConfiguration = app->config;
	LOGV(LOGTAG_INPUT,"Input: KeysHidden=%d,KeyboardConfig=%d", AConfiguration_getScreenLong(app->config),  AConfiguration_getKeyboard(app->config));	
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
	
	//engine.androidConfiguration = state->config;

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
	
	/*if (engine->sensorCollector != NULL)
	{
		engine->sensorCollector->disablesensors();
	}
	*/
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

//void sendMessagesToJNI(vector<OutgoingMessage*> messages)
//{
//	LOGD(LOGTAG_JNI,"Creating java message thread");
//	JavaMessageThread * thread = new JavaMessageThread(javaVM, clientObject, &incomingMutex, messages);
//	LOGD(LOGTAG_JNI,"Thread created.");
//}

void android_main(struct android_app* state)
{	
	LOG_INTRO();
	pthread_mutex_init(&incomingMutex,NULL);
	pthread_mutex_init(&outgoingMutex,NULL);

	//Initialize JNI-friendly variables
	jniDataVector.clear();
	connectedToServer = false;
	connectionString = "test";
	softKeyboardOpen = false;

	Engine mainEngine = Engine();
	initializeEngine(state, mainEngine);

	mainEngine.communicator = new ARCommunicator();
	
	AmplifyRunner myRunner = AmplifyRunner(&mainEngine);

	struct ARUserData myData;
	memset(&myData,0,sizeof(ARUserData));

	myData.engine = &mainEngine;
	myData.runner = &myRunner;


	state->userData = &myData;

	bool lastKeyboardState = softKeyboardOpen;

	
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
			//LOGV(LOGTAG_INPUT,"EventLoop: KeysHidden=%d,KeyboardConfig=%d", AConfiguration_getScreenLong(source->app->config),  AConfiguration_getKeyboard(source->app->config));	
		}

		
		//mainEngine.androidConfiguration = source->app->config;
		//int32_t keyboardConfig =;
		//AConfiguration * testConfig = AConfiguration_new();
		//int diffResult = AConfiguration_diff(testConfig, state->config);
		//LOGV(LOGTAG_INPUT,"DiffResult=%d",diffResult);
		//LOGV(LOGTAG_INPUT,"Engine: KeysHidden=%d,KeyboardConfig=%d", AConfiguration_getScreenLong(mainEngine.androidConfiguration),  AConfiguration_getKeyboard(mainEngine.androidConfiguration));			
		//LOGV(LOGTAG_INPUT,"Main: ContentRect[%d,%d,%d,%d]",state->contentRect.left,state->contentRect.top,state->contentRect.right,state->contentRect.bottom);
		//LOGV(LOGTAG_INPUT,"WindowSize=[%d,%d]",ANativeWindow_getWidth(state->window),ANativeWindow_get(state->window));


		if (softKeyboardOpen != lastKeyboardState)
		{
			mainEngine.inputHandler->SoftKeyboardChanged(softKeyboardOpen);
			lastKeyboardState = softKeyboardOpen;
		}

		//Check for messages in JNI queue
		if ( pthread_mutex_trylock(&incomingMutex) == 0)
		{
			mainEngine.communicator->SetConnected(connectedToServer);
			if (connectionString.length() == 0)
			{
				connectionString = mainEngine.communicator->GetConnectionString();
			}

			int count = 0;
			while (!jniDataVector.empty())
			{
				count++;
				mainEngine.communicator->AddIncomingMessage(jniDataVector.back());
				//LOGD(LOGTAG_MAIN,"Deleting JNI message. Count = %d",count);
				//delete jniDataVector.back();
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
		else
		{
			LOGW(LOGTAG_MAIN,"Exiting due to internal user command.");
			running = false;
		}
	}
	myRunner.~AmplifyRunner();
	shutdownEngine(&mainEngine);
	ANativeActivity_finish(state->activity);
}

