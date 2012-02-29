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

ARObjectMessage::ARObjectMessage(ARObject * _arObject)
{
	arObject = _arObject;
}

static jobject getPointAsJNIVector(JNIEnv * env, Point3f point)
{
	jclass wrapperClass = env->FindClass("com/amplifyreality/networking/model/Vector3");
	jmethodID initMethod = env->GetMethodID(wrapperClass,"<init>","(FFF)V");
	return env->NewObject(wrapperClass,initMethod,point.x,point.y,point.z);
}

jobject ARObjectMessage::getJavaObject(JNIEnv * env)
{
	jclass wrapperClass = env->FindClass("com/amplifyreality/networking/model/ARObject");
	jmethodID initMethod = env->GetMethodID(wrapperClass,"<init>","(java/lang/String;com/amplifyreality/networking/model/Vector3;com/amplifyreality/networking/model/Vector3;com/amplifyreality/networking/model/Vector3;)V");
	
	jstring name = env->NewStringUTF(arObject->objectID.c_str());
	jobject objectUpdate = env->NewObject(wrapperClass,initMethod,name, getPointAsJNIVector(env,arObject->position),getPointAsJNIVector(env,arObject->rotation),getPointAsJNIVector(env,arObject->scale));

	return objectUpdate;
}

jstring ARObjectMessage::GetDescription(JNIEnv * env)
{
	return env->NewStringUTF("ARObjectUpdate");
}