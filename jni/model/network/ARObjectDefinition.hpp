#ifndef AROBJECT_DEFINITION_HPP_
#define AROBJECT_DEFINITION_HPP_

#include <jni.h>
#include "LogDefinitions.h"
#include "AmplifyRealityGlobals.hpp"
#include <opencv2/core/core.hpp>

class ARObjectDefinition
{
public:
	std::string Name, ModelName;
	cv::Point3f Scale, Position, Rotation;

	ARObjectDefinition();
	ARObjectDefinition(std::string _Name, std::string _ModelName, cv::Point3f _Position, cv::Point3f _Rotation, cv::Point3f _Scale)
	{
		Name = _Name;
		ModelName = _ModelName;
		Position = _Position;
		Rotation = _Rotation;
		Scale = _Scale;
	};

	static ARObjectDefinition * FromJNIObject(JNIEnv * env, jobject arObject)
	{
		
		jclass arObjectClass = env->GetObjectClass(arObject);

		jfieldID nameField = env->GetFieldID(arObjectClass,"Name","Ljava/lang/String;");
		jfieldID modelNameField = env->GetFieldID(arObjectClass,"ModelName","Ljava/lang/String;");
		
		jfieldID positionField = env->GetFieldID(arObjectClass,"Position","Lcom/amplifyreality/networking/model/Vector3;");
		jfieldID rotationField = env->GetFieldID(arObjectClass,"Rotation","Lcom/amplifyreality/networking/model/Vector3;");
		jfieldID scaleField = env->GetFieldID(arObjectClass,"Scale","Lcom/amplifyreality/networking/model/Vector3;");

		jobject positionObject = env->GetObjectField(arObject,positionField);
		jobject rotationObject = env->GetObjectField(arObject,rotationField);
		jobject scaleObject = env->GetObjectField(arObject,scaleField);

		jstring nameObject = (jstring)env->GetObjectField(arObject,nameField);		
		jstring modelNameObject = (jstring)env->GetObjectField(arObject,modelNameField);

		std::string nameString = std::string(env->GetStringUTFChars(nameObject,JNI_FALSE));
		std::string modelNameString = std::string(env->GetStringUTFChars(modelNameObject,JNI_FALSE));	

		
		ARObjectDefinition * newARObject = 
			new ARObjectDefinition(nameString,modelNameString,
			FromJavaVector(env,positionObject),
			FromJavaVector(env,rotationObject),
			FromJavaVector(env,scaleObject));
				
		LOGI(LOGTAG_NETWORKING,"Created ARObjectDef with name=%s, modelname=%s, Position=[%f,%f,%f], Scale=[%f,%f,%f], Rotation=[%f,%f,%f]",
			newARObject->Name.c_str(),newARObject->ModelName.c_str(),newARObject->Position.x,newARObject->Position.y,newARObject->Position.z,
																	newARObject->Scale.x,newARObject->Scale.y,newARObject->Scale.z,
																	newARObject->Rotation.x,newARObject->Rotation.y,newARObject->Rotation.z);

		return newARObject;
	};

	static cv::Point3f FromJavaVector(JNIEnv * env, jobject vectorObject)
	{
		jclass vectorClass = env->GetObjectClass(vectorObject);

		jfieldID xField = env->GetFieldID(vectorClass,"X","F");
		jfieldID yField = env->GetFieldID(vectorClass,"Y","F");
		jfieldID zField = env->GetFieldID(vectorClass,"Z","F");

		float x = env->GetFloatField(vectorObject,xField);
		float y = env->GetFloatField(vectorObject,yField);
		float z = env->GetFloatField(vectorObject,zField);

		return cv::Point3f(x,y,z);
	};

};

#endif