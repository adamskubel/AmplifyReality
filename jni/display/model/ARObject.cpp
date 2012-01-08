#include "display/model/ARObject.hpp"

ARObject::ARObject()
{
	rotation = cv::Point3f();
	position = cv::Point3f();
	scale = cv::Point3f(1,1,1);
}

ARObject::ARObject(cv::Point3f _position)
{
	rotation = cv::Point3f();
	position = _position;
	scale = cv::Point3f(1,1,1);
}

ARObject::ARObject(cv::Point3f _position, cv::Point3f _rotation)
{
	rotation = _rotation;
	position = _position;
	scale = cv::Point3f(1,1,1);
}

ARObject::ARObject(cv::Point3f _position, cv::Point3f _rotation, cv::Point3f _scale)
{
	rotation = _rotation;
	position = _position;
	scale = _scale;
}