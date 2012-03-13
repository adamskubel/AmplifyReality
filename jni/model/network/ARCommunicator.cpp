#include "ARCommunicator.hpp"
#include <android_native_app_glue.h>
#include <jni.h>

ARCommunicator::ARCommunicator(jobject _arClientObject)
{
	state = CommunicatorStates::Ready;
	outgoingMessageQueue.clear();
	arClientObject = _arClientObject;
}

void ARCommunicator::SendMessage(OutgoingMessage * message)
{
	outgoingMessageQueue.push_back(message);
}

CommunicatorStates::CommunicatorState ARCommunicator::GetState()
{
	return state;
}

void ARCommunicator::ConnectTo(std::string _connectionString, std::string _userName, std::string _password)
{
	if (CanConnect())
	{
		LOGD(LOGTAG_NETWORKING,"Connecting next to %s",_connectionString.c_str());
		connectionString = _connectionString;
		state = CommunicatorStates::ConnectingNext;
		userName = _userName;
		password = _password;
	}
	else
	{
		LOGW(LOGTAG_NETWORKING,"Not yet ready to connect!");
	}
}

void ARCommunicator::Update(JavaVM * jvm)
{
	if (state == CommunicatorStates::ConnectingNext)
	{
		Connect(jvm);
	}
	else if (state == CommunicatorStates::Connected)
	{
		if (!outgoingMessageQueue.empty())
			SendMessages(jvm);
	}
}

void ARCommunicator::Connect(JavaVM * javaVM)
{
	JNIEnv * env = JNIUtils::GetJNIEnv(javaVM);		

	jstring j_connectString = env->NewStringUTF(connectionString.c_str());
	jstring j_userString = env->NewStringUTF(userName.c_str());
	jstring J_passString = env->NewStringUTF(password.c_str());

	jclass arClientClass =  env->GetObjectClass(arClientObject);
	jmethodID connectMethod = env->GetMethodID(arClientClass,"Connect","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");


	LOGD(LOGTAG_NETWORKING,"Connecting to %s as %s",connectionString.c_str(),userName.c_str());
	jint jResult = env->CallIntMethod(arClientObject,connectMethod,j_connectString,j_userString,J_passString);

	int result = (int)jResult;
	LOGD(LOGTAG_NETWORKING,"Connection result = %d",result);

	if (result == 0)
		state = CommunicatorStates::Connected;
	else if (result == 3)
		state = CommunicatorStates::AuthFailed;	
	else if (result == -1)
		state = CommunicatorStates::InvalidHost;	
	else
		state = CommunicatorStates::ConnectFailed;

	LOGD(LOGTAG_JNI,"Detaching thread");
	javaVM->DetachCurrentThread();
}

void ARCommunicator::SendMessages(JavaVM * javaVM) 
{
	struct timespec start,end;
	SET_TIME(&start);

	JNIEnv * env = JNIUtils::GetJNIEnv(javaVM);
	
	jclass wrapperClass = JNIUtils::GetClass("com/amplifyreality/networking/message/NativeMessage");
	jmethodID initMethod = env->GetMethodID(wrapperClass,"<init>","(Ljava/lang/String;Ljava/lang/Object;)V");
	jobjectArray returnArray = env->NewObjectArray(outgoingMessageQueue.size(),wrapperClass,NULL);

	int i= 0;
	while (!outgoingMessageQueue.empty())
	{	
		if (outgoingMessageQueue.back() == NULL)
			continue;

		jstring description = outgoingMessageQueue.back()->GetDescription(env);
		jobject dataObject = outgoingMessageQueue.back()->getJavaObject(env);
		
		jobject returnObject = env->NewObject(wrapperClass,initMethod,description,dataObject);
		if (JNIUtils::LogJNIError(env)) continue;
		env->SetObjectArrayElement(returnArray,i++,returnObject);
		if (JNIUtils::LogJNIError(env)) continue;

		//LOGI(LOGTAG_JNI,"Deleting message %d",i);
		//delete outgoingMessageQueue.back(); //Why does this cause crash?!!
		outgoingMessageQueue.pop_back();
	}

	jclass arClientClass =  env->GetObjectClass(arClientObject);
	jmethodID sendMethod = env->GetMethodID(arClientClass,"QueueNativeMessages","([Ljava/lang/Object;)V");
	env->CallVoidMethod(arClientObject,sendMethod,returnArray);

	javaVM->DetachCurrentThread();
	
	SET_TIME(&end);
	double time = calc_time_double(start,end);
	LOGI(LOGTAG_JNI,"Message sending took %lf us",time);
}


bool ARCommunicator::IsConnected()
{
	return (state == CommunicatorStates::Connected);
}

bool ARCommunicator::CanConnect()
{
	return (state == CommunicatorStates::Ready || state == CommunicatorStates::AuthFailed || state == CommunicatorStates::InvalidHost || state == CommunicatorStates::ConnectFailed);
}


void ARCommunicator::AddIncomingMessage(IncomingMessage * message)
{
	if (message != NULL)
		incomingMessageQueue.push_back(message);
}


bool ARCommunicator::FilterMessages(std::string filter, std::vector<IncomingMessage*> & newMsgs)
{
	if (incomingMessageQueue.empty())
		return false;
/*
	LOGD(LOGTAG_NETWORKING,"%d messages in queue. Returning messages with tag %s", incomingMessageQueue.size(), filter.c_str());*/
	bool found = false;
	for (int i=0;i<incomingMessageQueue.size();i++)
	{
		if (incomingMessageQueue.at(i)->GetAction().compare(filter) == 0)
		{
			found = true;
			newMsgs.push_back(incomingMessageQueue.at(i));
			incomingMessageQueue.erase(incomingMessageQueue.begin() + i);
			i--;
		}
	}		
	return found;
}

