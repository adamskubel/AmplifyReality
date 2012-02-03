#ifndef WAVEFRONT_MODEL_HPP_
#define WAVEFRONT_MODEL_HPP_

#include <jni.h>
#include "LogDefinitions.h"

class WavefrontModel : public IncomingMessage
{
public:
	std::string ModelName;
	std::string ModelData;

	std::string GetAction()
	{
		return "Wavefront";
	}

	WavefrontModel()
	{
	}

	WavefrontModel(std::string _ModelName, std::string _ModelData)
	{
		ModelName = _ModelName;
		ModelData = _ModelData;
	}

	static WavefrontModel * FromJNI(JNIEnv * env, jobject waveObject)
	{
		WavefrontModel * newModel = new WavefrontModel();

		jclass wavefrontClass = env->GetObjectClass(waveObject);

		jfieldID nameField = env->GetFieldID(wavefrontClass,"Name","Ljava/lang/String;");
		jstring nameObject = (jstring)env->GetObjectField(waveObject,nameField);		
		
		jfieldID dataField = env->GetFieldID(wavefrontClass,"ObjData","Ljava/lang/String;");
		jstring dataStringObject = (jstring)env->GetObjectField(waveObject,nameField);		

		newModel->ModelName = std::string(env->GetStringUTFChars(nameObject,JNI_FALSE));				
		newModel->ModelData = std::string(env->GetStringUTFChars(dataStringObject,JNI_FALSE));

		return newModel;
	}
};

#endif