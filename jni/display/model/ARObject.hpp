#include "LogDefinitions.h"
#include "display/opengl/GLObject.hpp"
#include "display/objloader/objLoader.h"
#include "util/GeometryUtil.hpp"


#ifndef AROBJECT_HPP_
#define AROBJECT_HPP_


class ARObject
{
public:
	GLObject * glObject;

	ARObject(GLObject * _glObject);
	ARObject(GLObject * _glObject, cv::Point3f position);
	ARObject(GLObject * _glObject, cv::Point3f position,cv::Point3f rotation);
	ARObject(GLObject * _glObject, cv::Point3f position,cv::Point3f rotation, cv::Point3f scale);
	~ARObject();

	cv::Point3f position;
	cv::Point3f rotation;
	cv::Point3f scale;

	float BoundingSphereRadius;

	static ARObject FromObjFile(objLoader & objData);
	
};

class ARObjectDistanceSort_Ascending
{
public:
	ARObjectDistanceSort_Ascending(cv::Point3f point)
	{
		refPoint = point;
	}

	bool operator()(const ARObject * obj1, const ARObject * obj2)
	{
		return GetSquaredDistance(obj1->position,refPoint) < GetSquaredDistance(obj2->position,refPoint);
	}

private:
	cv::Point3f refPoint;
};
#endif
