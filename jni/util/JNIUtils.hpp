#ifndef JNI_UTILS_HPP_
#define JNI_UTILS_HPP_

#include <jni.h>
#include <map>
#include <LogDefinitions.h>

using namespace std;

class JNIUtils
{
public:
	static JNIEnv * GetJNIEnv(JavaVM * lJavaVM);
	static bool LogJNIError(JNIEnv * env);
	static void AddClass(JNIEnv * env, std::string qualifiedName);
	static jclass GetClass(std::string qualifiedName);
	//static jmethodID GetMethodID(std::string methodName, 
private:
	static std::map<std::string,jclass> javaClassMap;
};


#endif