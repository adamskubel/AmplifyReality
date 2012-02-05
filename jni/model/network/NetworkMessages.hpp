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
	virtual jstring GetDescription(JNIEnv * env)
	{
		return env->NewStringUTF("Base");
	}

	//virtual jstring GetDataString(JNIEnv * env)
	//{
	//	return env->NewStringUTF("BaseString");
	//}

	virtual jobject getJavaObject(JNIEnv * env)
	{
		return env->NewStringUTF("null");
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

	jstring GetDescription(JNIEnv * env)
	{
		return env->NewStringUTF("String");
	}

	//jstring GetDataString(JNIEnv * env)
	//{
	//	return env->NewStringUTF(StringData->c_str());
	//}

		
	jobject getJavaObject(JNIEnv * env)
	{
		return  env->NewStringUTF(StringData->c_str());
	}

	std::string * StringData;
};



#endif