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
	//LOGD(LOGTAG_JNI,"Finding vector3 jclass");
	jclass wrapperClass = env->FindClass("com/amplifyreality/networking/model/Vector3");
	//LOGD(LOGTAG_JNI,"Finding vector3 init method");
	jmethodID initMethod = env->GetMethodID(wrapperClass,"<init>","(FFF)V");
	return env->NewObject(wrapperClass,initMethod,(jfloat)point.x,(jfloat)point.y,(jfloat)point.z);
}

jobject ARObjectMessage::getJavaObject(JNIEnv * env)
{
	jclass arObjectClass = env->FindClass("com/amplifyreality/networking/model/ARObject");
	jmethodID initMethod = env->GetMethodID(arObjectClass,"<init>","(Ljava/lang/String;Lcom/amplifyreality/networking/model/Vector3;Lcom/amplifyreality/networking/model/Vector3;)V");
	
	jstring name = env->NewStringUTF(arObject->objectID.c_str());
	//LOGD(LOGTAG_JNI,"Creating java ARObject. InitMethod=%d",initMethod);
	jobject positionVector = getPointAsJNIVector(env,arObject->position);
	jobject rotationVector = getPointAsJNIVector(env,arObject->rotation);
	//LOGD(LOGTAG_JNI,"Vectors complete");
	jobject objectUpdate = NULL;
	//try
	//{
		objectUpdate = env->NewObject(arObjectClass,initMethod,name,positionVector,rotationVector);
	//}
	//catch (std::exception & e)
	//{
	//	LOGW(LOGTAG_JNI,"Error creating ARObject: %s", e.what());
	//}
	//LOGD(LOGTAG_JNI,"ARObject created");
	return objectUpdate;
}

jstring ARObjectMessage::GetDescription(JNIEnv * env)
{
	return env->NewStringUTF("ARObjectUpdate");
}