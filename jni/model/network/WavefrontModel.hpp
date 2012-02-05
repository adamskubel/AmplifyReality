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

	WavefrontModel(WavefrontModel & copyModel)
	{
		LOGD(LOGTAG_NETWORKING,"Created copy of wavefrontmodel");
		ModelName = string(copyModel.ModelName);
		ModelData = string(copyModel.ModelData);
	}

	~WavefrontModel()
	{
		LOGD(LOGTAG_NETWORKING,"Deleting wavefront model");
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
		jstring dataStringObject = (jstring)env->GetObjectField(waveObject,dataField);		

		newModel->ModelName = std::string(env->GetStringUTFChars(nameObject,JNI_FALSE));				
		newModel->ModelData = std::string(env->GetStringUTFChars(dataStringObject,JNI_FALSE));

		LOGD(LOGTAG_NETWORKING,"Created wavefront model message. Name=%s",newModel->ModelName.c_str());

		return newModel;
	}
};

#endif