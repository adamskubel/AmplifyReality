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
