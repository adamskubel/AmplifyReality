#include "display/opengl/OpenGL.hpp"
#include "display/opengl/GLObject.hpp"
#include "userinterface/uimodel/UIElement.hpp"

#ifndef OPENGL_HELPER_HPP_
#define OPENGL_HELPER_HPP_

#define FIXED(value) OpenGLHelper::floatToFixed(value)

//Class containing static OpenGL related helper methods
class OpenGLHelper
{
public:	
	static void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz);
	static void gluPerspective(GLfloat fovy, GLfloat aspectRatio, GLfloat zNear, GLfloat zFar);
	static void configureLightAndMaterial();
	static TexturedGLObject* CreateTexturedQuad(int textureWidth, int textureHeight, int size);
	static void drawGLObject(GLObject *object);
	static void freeGLObject(GLObject *object);
	static long floatToFixed(float value);
	static ColorGLObject* CreateCube(int size);
	static ColorGLObject* CreateSolidColorCube(int size, cv::Scalar color);
	static ColorGLObject* CreateMultiColorCube(int size);
	
	static void PopulateColors(ColorGLObject * colorObject, vector<cv::Scalar> * colors);
	static void PopulateVertices(GLObject * glObject, vector<cv::Point3f> * vertices);
};

#endif