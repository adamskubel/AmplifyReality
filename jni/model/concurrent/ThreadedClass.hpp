#ifndef THREADED_CLASS_HPP_
#define THREADED_CLASS_HPP_

#include <pthread.h>
#include "LogDefinitions.h"
#include <jni.h>
#include "model/network/NetworkMessages.hpp"

class JavaMessageThread
{
public:
	JavaMessageThread(JavaVM * _javaVM, jobject _clientObject, pthread_mutex_t * jniMutex, vector<OutgoingMessage*> & _outgoingJNIDataVector)
	{
		javaVM = _javaVM;
		threadMutex = jniMutex;
		clientObject = _clientObject;
		outgoingJNIDataVector = _outgoingJNIDataVector; //copy vector
		//javaEnv = jniEnv;		
		pthread_create(&threadObject, 0, &JavaMessageThread::StartThread, this);
	};
	

private:
	//JNIEnv * javaEnv;
	jobject clientObject;
	pthread_mutex_t * threadMutex;
	pthread_t threadObject;
	JavaVM * javaVM;
	vector<OutgoingMessage*> outgoingJNIDataVector;

	static void * StartThread(void * thread)
	{
		LOGD(LOGTAG_JNI,"Starting thread.");
		reinterpret_cast<JavaMessageThread *>(thread)->run();
	}


	//Populate objects and call JNI method to send them
	void run()
	{
		LOGI(LOGTAG_JNI,"Thread running, %d messages to send.",outgoingJNIDataVector.size());
		pthread_mutex_lock(threadMutex);

		LOGI(LOGTAG_JNI,"Attach env to JVM");
		JNIEnv * javaEnv;// = new JNIEnv();
		jint result = javaVM->AttachCurrentThread(&javaEnv,NULL);
		//jint result = javaVM->GetEnv((void**)&javaEnv,JNI_VERSION_1_6);
		LOGI(LOGTAG_JNI,"Result = %d",result);
		LOGI(LOGTAG_JNI,"Version = %d",javaEnv->GetVersion());

		
		LOGI(LOGTAG_JNI,"Looking for class in JNI env");
		jclass wrapperClass = javaEnv->FindClass("com/amplifyreality/networking/message/NativeMessage");
		LOGI(LOGTAG_JNI,"Got native message class");

		jmethodID initMethod = javaEnv->GetMethodID(wrapperClass,"<init>","(Ljava/lang/String;Ljava/lang/Object;)V");

		LOGI(LOGTAG_JNI,"Creating new Object[]");
		jobjectArray returnArray = javaEnv->NewObjectArray(outgoingJNIDataVector.size(),wrapperClass,NULL);
		LOGI(LOGTAG_JNI,"Complete.");
		int i= 0;
		while (!outgoingJNIDataVector.empty())
		{	
			jstring description = outgoingJNIDataVector.back()->GetDescription(javaEnv);
			jobject dataObject = outgoingJNIDataVector.back()->getJavaObject(javaEnv);
			LOGI(LOGTAG_JNI,"Set properties of NativeMessage");
			jobject returnObject = javaEnv->NewObject(wrapperClass,initMethod,description,dataObject);
			javaEnv->SetObjectArrayElement(returnArray,i++,returnObject);

			//	delete outgoingJNIDataVector.back();
			outgoingJNIDataVector.pop_back();
		}
		
		jclass clientClassType = javaEnv->FindClass("com/amplifyreality/networking/ARClient");
		LOGI(LOGTAG_JNI,"Got ARClient class");
		jmethodID sendMethod = javaEnv->GetMethodID(clientClassType,"QueueNativeMessages","([Ljava/lang/Object;)V");
		LOGI(LOGTAG_JNI,"Calling send method");
		javaEnv->CallVoidMethod(clientObject,sendMethod,returnArray);
		pthread_mutex_unlock(threadMutex);

		LOGI(LOGTAG_JNI,"Suiciding!");
		delete this;
	}

};


#endif