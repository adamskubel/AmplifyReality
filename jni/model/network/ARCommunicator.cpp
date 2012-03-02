#include "ARCommunicator.hpp"
#include <android_native_app_glue.h>
#include <jni.h>


ARCommunicator::ARCommunicator()
{
	state = CommunicatorStates::Starting;
	outgoingMessageQueue.clear();
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

static JNIEnv * GetJNIEnv(JavaVM * lJavaVM)
{
	LOGV(LOGTAG_NETWORKING,"Getting JVM");
	jint lResult; 
	jint lFlags = 0; 
	JNIEnv * lJNIEnv = new JNIEnv();
	JavaVMAttachArgs lJavaVMAttachArgs; 
	lJavaVMAttachArgs.version = JNI_VERSION_1_6; 
	lJavaVMAttachArgs.name = "NativeThread"; 
	lJavaVMAttachArgs.group = NULL; 
	lResult=lJavaVM->AttachCurrentThread(&lJNIEnv, 
		&lJavaVMAttachArgs); 
	if (lResult == JNI_ERR) { 
		LOGE(LOGTAG_NETWORKING,"Error getting JVM");
		return NULL; 
	} 	
	LOGV(LOGTAG_NETWORKING,"Returning JVM");
	return lJNIEnv;
}

void ARCommunicator::SetClientObject(jobject _arClientObject)
{
	arClientObject = _arClientObject;
	if (arClientObject != NULL)
	{
		state = CommunicatorStates::Ready;
		LOGD(LOGTAG_NETWORKING,"Client object set");
	}
	else
	{
		LOGW(LOGTAG_NETWORKING,"Null client object set!");
	}
}

void ARCommunicator::Update(JavaVM * jvm)
{
	if (state == CommunicatorStates::ConnectingNext)
	{
		JNIEnv * env = GetJNIEnv(jvm);		
		
		//LOGD(LOGTAG_NETWORKING,"String conversion");
		jstring j_connectString = env->NewStringUTF(connectionString.c_str());
		jstring j_userString = env->NewStringUTF(userName.c_str());
		jstring J_passString = env->NewStringUTF(password.c_str());

		//LOGD(LOGTAG_NETWORKING,"Getting java connection class");
		jclass arClientClass =  env->GetObjectClass(arClientObject);
		//LOGD(LOGTAG_NETWORKING,"Getting java connection method");
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
	}
}

void ARCommunicator::GetOutgoingMessages(std::vector<OutgoingMessage*> & newMsgQueue)
{
	while (outgoingMessageQueue.empty() == false)
	{
		newMsgQueue.push_back(outgoingMessageQueue.back());
		outgoingMessageQueue.pop_back();
	}
}

bool ARCommunicator::HasOutgoingMessages()
{
	return (!outgoingMessageQueue.empty());
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

	LOGD(LOGTAG_NETWORKING,"%d messages in queue. Returning messages with tag %s", incomingMessageQueue.size(), filter.c_str());
	bool found = false;
	for (int i=0;i<incomingMessageQueue.size();i++)
	{
		if (incomingMessageQueue.at(i)->GetAction().compare(filter) == 0)
		{
			found = true;
			newMsgs.push_back(incomingMessageQueue.at(i));
			//LOGV(LOGTAG_NETWORKING,"Erasing msg at i=%d",i);
			incomingMessageQueue.erase(incomingMessageQueue.begin() + i);
			i--;
		}
	}		
	//LOGD(LOGTAG_NETWORKING,"%d matching messages found", newMsgs.size());
	return found;
}

