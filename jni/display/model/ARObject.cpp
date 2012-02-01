#include "display/model/ARObject.hpp"

ARObject::ARObject(GLObject * _glObject)
{
	glObject = _glObject;
	rotation = cv::Point3f(0,0,0);
	position = cv::Point3f(0,0,0);
	scale = cv::Point3f(1,1,1);
}

ARObject::ARObject(GLObject * _glObject, cv::Point3f _position)
{
	glObject = _glObject;
	rotation = cv::Point3f(0,0,0);
	position = _position;
	scale = cv::Point3f(1,1,1);
}

ARObject::ARObject(GLObject * _glObject, cv::Point3f _position, cv::Point3f _rotation)
{
	glObject = _glObject;
	rotation = _rotation;
	position = _position;
	scale = cv::Point3f(1,1,1);
}

ARObject::ARObject(GLObject * _glObject, cv::Point3f _position, cv::Point3f _rotation, cv::Point3f _scale)
{
	glObject = _glObject;
	rotation = _rotation;
	position = _position;
	scale = _scale;
}

ARObject::~ARObject()
{
	delete glObject;
}

//ARObject ARObject::FromObjFile(objLoader & loader)
//{
//	GLObject * object = new ColorGLObject(loader.vertexCount);
//	
//	for (int i=0;i<loader.vertexCount;i++)
//	{
//		double * vertex = (loader.vertexList[0])[i].e;
//		object->AddVertex(cv::Point3f(vertex[0],vertex[1],vertex[2]),i);
//	}
//
//
//}