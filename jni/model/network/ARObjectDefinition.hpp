#ifndef AROBJECT_DEFINITION_HPP_
#define AROBJECT_DEFINITION_HPP_

#include <jni.h>
#include "LogDefinitions.h"
#include "AmplifyRealityGlobals.hpp"
#include <opencv2/core/core.hpp>
#include "NetworkMessages.hpp"

class ARObjectDefinition : public IncomingMessage
{
public:
	std::string Name, ModelName, Action;
	cv::Point3f Scale, Position, Rotation;
	float BoundingSphereRadius;

	std::string GetAction();

	ARObjectDefinition();
	ARObjectDefinition(std::string _Name, cv::Point3f _Position, cv::Point3f _Rotation);
	ARObjectDefinition(std::string _Name, std::string _ModelName, cv::Point3f _Position, cv::Point3f _Rotation, cv::Point3f _Scale, float _BoundingShereRadius);
	static ARObjectDefinition * FromJNIObject(JNIEnv * env, jobject arObject);
	static ARObjectDefinition * FromJNIObject_Update(JNIEnv * env, jobject arObject);
	static cv::Point3f FromJavaVector(JNIEnv * env, jobject vectorObject);

};

#endif