#include "ARObjectDefinition.hpp"

ARObjectDefinition::ARObjectDefinition()
{
	;
}

std::string ARObjectDefinition::GetAction()
{
	return "Action";
}

ARObjectDefinition::ARObjectDefinition(std::string _Name, cv::Point3f _Position, cv::Point3f _Rotation)
{
	Name = _Name;
	Position = _Position;
	Rotation = _Rotation;	
}

ARObjectDefinition::ARObjectDefinition(std::string _Name, std::string _ModelName, cv::Point3f _Position, cv::Point3f _Rotation, cv::Point3f _Scale, float _BoundingShereRadius)
{
	Name = _Name;
	ModelName = _ModelName;
	Position = _Position;
	Rotation = _Rotation;
	BoundingSphereRadius = _BoundingShereRadius;
	Scale = _Scale;
}

ARObjectDefinition * ARObjectDefinition::FromJNIObject(JNIEnv * env, jobject arObject)
{

	jclass arObjectClass = env->GetObjectClass(arObject);

	jfieldID nameField = env->GetFieldID(arObjectClass,"Name","Ljava/lang/String;");
	jfieldID modelNameField = env->GetFieldID(arObjectClass,"ModelName","Ljava/lang/String;");

	jfieldID positionField = env->GetFieldID(arObjectClass,"Position","Lcom/amplifyreality/networking/model/Vector3;");
	jfieldID rotationField = env->GetFieldID(arObjectClass,"Rotation","Lcom/amplifyreality/networking/model/Vector3;");
	jfieldID scaleField = env->GetFieldID(arObjectClass,"Scale","Lcom/amplifyreality/networking/model/Vector3;");
	jfieldID boundingField = env->GetFieldID(arObjectClass,"BoundingSphereRadius","F");


	jfloat boundingRadius = env->GetFloatField(arObject,boundingField);
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
		FromJavaVector(env,scaleObject),
		(float)boundingRadius);

	newARObject->Action = "Create";

	LOGI(LOGTAG_NETWORKING,"Created ARObjectDef with name=%s, modelname=%s, Position=[%f,%f,%f], Scale=[%f,%f,%f], Rotation=[%f,%f,%f], SphereRadius=%f",
		newARObject->Name.c_str(),newARObject->ModelName.c_str(),newARObject->Position.x,newARObject->Position.y,newARObject->Position.z,
		newARObject->Scale.x,newARObject->Scale.y,newARObject->Scale.z,
		newARObject->Rotation.x,newARObject->Rotation.y,newARObject->Rotation.z,boundingRadius);

	return newARObject;
}

//Only get updateable fields
ARObjectDefinition * ARObjectDefinition::FromJNIObject_Update(JNIEnv * env, jobject arObject)
{

	jclass arObjectClass = env->GetObjectClass(arObject);

	jfieldID nameField = env->GetFieldID(arObjectClass,"Name","Ljava/lang/String;");

	jfieldID positionField = env->GetFieldID(arObjectClass,"Position","Lcom/amplifyreality/networking/model/Vector3;");
	jfieldID rotationField = env->GetFieldID(arObjectClass,"Rotation","Lcom/amplifyreality/networking/model/Vector3;");
	jobject positionObject = env->GetObjectField(arObject,positionField);
	jobject rotationObject = env->GetObjectField(arObject,rotationField);

	jstring nameObject = (jstring)env->GetObjectField(arObject,nameField);			
	std::string nameString = std::string(env->GetStringUTFChars(nameObject,JNI_FALSE));
	
	ARObjectDefinition * newARObject = 
		new ARObjectDefinition(nameString,FromJavaVector(env,positionObject),FromJavaVector(env,rotationObject));
	newARObject->Action = "Update";

	return newARObject;
}

cv::Point3f ARObjectDefinition::FromJavaVector(JNIEnv * env, jobject vectorObject)
{
	jclass vectorClass = env->GetObjectClass(vectorObject);

	jfieldID xField = env->GetFieldID(vectorClass,"X","F");
	jfieldID yField = env->GetFieldID(vectorClass,"Y","F");
	jfieldID zField = env->GetFieldID(vectorClass,"Z","F");

	float x = env->GetFloatField(vectorObject,xField);
	float y = env->GetFloatField(vectorObject,yField);
	float z = env->GetFloatField(vectorObject,zField);

	return cv::Point3f(x,y,z);
}

