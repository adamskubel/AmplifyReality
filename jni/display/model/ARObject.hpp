#include "LogDefinitions.h"
#include "display/opengl/GLObject.hpp"


#ifndef AROBJECT_HPP_
#define AROBJECT_HPP_
class ARObject : public GLObject
{
public:
	ARObject();
	ARObject(cv::Point3f position);
	ARObject(cv::Point3f position,cv::Point3f rotation);
	ARObject(cv::Point3f position,cv::Point3f rotation, cv::Point3f scale);
	cv::Point3f position;
	cv::Point3f rotation;
	cv::Point3f scale;


};
#endif
