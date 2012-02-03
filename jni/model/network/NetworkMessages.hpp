#ifndef BASE_MESSAGE_HPP_
#define BASE_MESSAGE_HPP_

using namespace std;


class IncomingMessage
{
public:
	virtual std::string GetAction()
	{
		return std::string("Base");
	}
};


class OutgoingMessage
{
public:
	jstring GetActionString(JNIEnv * env)
	{
		return env->NewStringUTF("Base");
	}

	virtual jobject * getJavaObject(JNIEnv * env)
	{
		return NULL;
	}

};

class StringMessage : public IncomingMessage, public OutgoingMessage
{
public:
	StringMessage(std::string * _stringData)
	{
		StringData = _stringData;
	}

	StringMessage(std::string _stringData)
	{
		StringData = new std::string(_stringData);
	}

	~StringMessage()
	{
		delete StringData;
	}

	std::string GetAction()
	{
		return std::string("String");
	}

	jstring GetActionString(JNIEnv * env)
	{
		return env->NewStringUTF("String");
	}

		
	jobject * getJavaObject(JNIEnv * env)
	{
		return (jobject*) env->NewStringUTF(StringData->c_str());
	}

	std::string * StringData;
};

class ResourceRequestMessage : public OutgoingMessage
{
public:
	ResourceRequestMessage(string _resourceName)
	{
		resourceName = new string(_resourceName);
	}

	jstring GetActionString(JNIEnv * env)
	{
		return env->NewStringUTF("ResourceRequest");
	}


	jobject * getJavaObject(JNIEnv * env)
	{
		return (jobject*) env->NewStringUTF(resourceName->c_str());
	}

	~ResourceRequestMessage()
	{
		delete resourceName;
	}

private:
	string * resourceName;
};



#endif