#ifndef JNI_UTILS_HPP_
#define JNI_UTILS_HPP_

#include <jni.h>
#include <LogDefinitions.h>

static JNIEnv * GetJNIEnv(JavaVM * lJavaVM)
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
		LOGE(LOGTAG_JNI,"Error getting JVM");
		return NULL; 
	} 	
	return lJNIEnv;
}

#endif