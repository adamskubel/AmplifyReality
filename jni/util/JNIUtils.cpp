#include "JNIUtils.hpp"

JNIEnv * JNIUtils::GetJNIEnv(JavaVM * lJavaVM)
{
	jint lResult; 
	jint lFlags = 0; 
	JNIEnv * lJNIEnv = new JNIEnv();
	JavaVMAttachArgs lJavaVMAttachArgs; 
	lJavaVMAttachArgs.version = JNI_VERSION_1_6; 
	lJavaVMAttachArgs.name = "NativeThread"; 
	lJavaVMAttachArgs.group = NULL; 
	lResult=lJavaVM->AttachCurrentThread(&lJNIEnv,&lJavaVMAttachArgs); 
	if (lResult == JNI_ERR)
	{ 
		LOGW(LOGTAG_JNI,"Error getting JVM");
		return NULL; 
	} 	
	return lJNIEnv;
}

bool JNIUtils::LogJNIError(JNIEnv * env)
{
	if (env->ExceptionCheck()) 
	{
		jthrowable e = env->ExceptionOccurred();
		env->ExceptionClear();
		jclass eclass = env->GetObjectClass(e);
		jmethodID toStringMethod = env->GetMethodID(eclass, "toString", "()Ljava/lang/String;");
		jstring jErrorMsg = (jstring) env->CallObjectMethod(e, toStringMethod);
		LOGW(LOGTAG_JNI,"JNI Error: %s",env->GetStringUTFChars(jErrorMsg,NULL));
		return true;	
	}
	return false;
}

jclass JNIUtils::GetClass(std::string qualifiedName)
{
	if (javaClassMap.count(qualifiedName) > 0)
		return javaClassMap[qualifiedName];
	else
		return NULL;
}

void JNIUtils::AddClass(JNIEnv * env, std::string qualifiedName)
{
	if (javaClassMap.count(qualifiedName) == 0)
	{
		jclass newClass = env->FindClass(qualifiedName.c_str());
		javaClassMap.insert(pair<string,jclass>(qualifiedName,newClass));
	}
}
