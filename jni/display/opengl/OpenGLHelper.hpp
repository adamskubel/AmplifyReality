#include "display/opengl/OpenGL.hpp"
#include "display/opengl/GLObject.hpp"
#include "userinterface/uimodel/UIElement.hpp"

#ifndef OPENGL_HELPER_HPP_
#define OPENGL_HELPER_HPP_

using namespace cv;

//Class containing static OpenGL related helper methods
class OpenGLHelper
{
public:	
	static void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz);

	static void gluPerspective(cv::Mat & matrix, GLfloat fovy, GLfloat aspectRatio, GLfloat zNear, GLfloat zFar);
	static void createFrustum(cv::Mat & matrix, float left, float right, float top, float bottom, float zFar, float zNear);
	static void translate(cv::Mat & matrix,cv::Point3f point);
	static void rotate(cv::Mat & matrix, float rotation, cv::Point3f rotationVector);
	static void scale(cv::Mat & matrix, cv::Point3f scale);
	static void createOrtho(Mat & matrix, float left, float right, float bottom, float top, float nearVal, float farVal);

	static void configureLightAndMaterial();
	static TexturedGLObject* CreateTexturedQuad(int textureWidth, int textureHeight, int size);
	static ColorGLObject* CreateCube(int size);
	static ColorGLObject* CreateSolidColorCube(int size, cv::Scalar color);
	static ColorGLObject* CreateMultiColorCube(int size);
	
	static void PopulateColors(ColorGLObject * colorObject, vector<cv::Scalar> * colors);
	static void PopulateVertices(GLObject * glObject, vector<cv::Point3f> * vertices);
};

#endif