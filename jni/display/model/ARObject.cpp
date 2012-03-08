#include "display/model/ARObject.hpp"


ARObject::ARObject(GLObject * _glObject, cv::Point3f _position, cv::Point3f _rotation, cv::Point3f _scale)
{
	glObject = _glObject;
	rotation = _rotation;
	position = _position;
	scale = _scale;
	BoundingSphereRadius = 10.0f;
}

ARObject::~ARObject()
{
	delete glObject;
}

ARObjectMessage::ARObjectMessage(ARObject * _arObject, bool _createNew)
{
	createNew = _createNew;
	arObject = _arObject;
}

static jobject getPointAsJNIVector(JNIEnv * env, Point3f point)
{	
	//LOGD(LOGTAG_JNI,"Finding vector3 jclass");
	jclass wrapperClass = env->FindClass("com/amplifyreality/networking/model/Vector3");
	//LOGD(LOGTAG_JNI,"Finding vector3 init method");
	jmethodID initMethod = env->GetMethodID(wrapperClass,"<init>","(FFF)V");
	return env->NewObject(wrapperClass,initMethod,(jfloat)point.x,(jfloat)point.y,(jfloat)point.z);
}

jobject ARObjectMessage::getJavaObject(JNIEnv * env)
{
	if (createNew)
	{
		LOGD(LOGTAG_JNI,"Creating new ARObject, name = %s",arObject->objectID.c_str());

		jclass arObjectClass = env->FindClass("com/amplifyreality/networking/model/ARObject");
		jmethodID initMethod = env->GetMethodID(arObjectClass,"<init>","(Ljava/lang/String;Ljava/lang/String;Lcom/amplifyreality/networking/model/Vector3;Lcom/amplifyreality/networking/model/Vector3;)V");

		jstring name = env->NewStringUTF(arObject->objectID.c_str());
		jobject positionVector = getPointAsJNIVector(env,arObject->position);
		jobject rotationVector = getPointAsJNIVector(env,arObject->rotation);
		jobject objectUpdate = NULL;
		jstring modelName = env->NewStringUTF("Cube");

		objectUpdate = env->NewObject(arObjectClass,initMethod,name,modelName,positionVector,rotationVector);
		return objectUpdate;
	}
	else
	{
		jclass arObjectClass = env->FindClass("com/amplifyreality/networking/model/ARObject");
		jmethodID initMethod = env->GetMethodID(arObjectClass,"<init>","(Ljava/lang/String;Lcom/amplifyreality/networking/model/Vector3;Lcom/amplifyreality/networking/model/Vector3;)V");

		jstring name = env->NewStringUTF(arObject->objectID.c_str());
		jobject positionVector = getPointAsJNIVector(env,arObject->position);
		jobject rotationVector = getPointAsJNIVector(env,arObject->rotation);
		jobject objectUpdate = NULL;

		objectUpdate = env->NewObject(arObjectClass,initMethod,name,positionVector,rotationVector);
		return objectUpdate;
	}
}

jstring ARObjectMessage::GetDescription(JNIEnv * env)
{
	return env->NewStringUTF("ARObjectUpdate");
}