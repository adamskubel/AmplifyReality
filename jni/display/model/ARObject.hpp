#include "LogDefinitions.h"
#include "display/opengl/GLObject.hpp"


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
	
};
#endif
