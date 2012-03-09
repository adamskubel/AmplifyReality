
#ifndef AROBJECT_HPP_
#define AROBJECT_HPP_

#include "LogDefinitions.h"
#include "display/opengl/GLObject.hpp"
#include "display/objloader/objLoader.h"
#include "util/GeometryUtil.hpp"
#include "model/network/NetworkMessages.hpp"
#include "util/JNIUtils.hpp"

using namespace cv;



class ARObject
{
public:
	GLObject * glObject;

	ARObject(GLObject * _glObject, Point3f position = Point3f(0,0,0),Point3f rotation = Point3f(0,0,0), Point3f scale = Point3f(1,1,1));
	~ARObject();

	Point3f position;
	Point3f rotation;
	Point3f scale;

	float BoundingSphereRadius;

	std::string objectID;

	static std::string generateObjectMessage();

};

class ARObjectMessage : public OutgoingMessage
{
public:
	ARObjectMessage(ARObject * object, bool createNew = false);
	jstring GetDescription(JNIEnv * env);
	jobject getJavaObject(JNIEnv * env);
private:
	ARObject * arObject;
	bool createNew;
};

class ARObjectDistanceSort_Ascending
{
public:
	ARObjectDistanceSort_Ascending(Point3f point)
	{
		refPoint = point;
	}

	bool operator()(const ARObject * obj1, const ARObject * obj2)
	{
		return GetSquaredDistance(obj1->position,refPoint) < GetSquaredDistance(obj2->position,refPoint);
	}

private:
	Point3f refPoint;
};
#endif
