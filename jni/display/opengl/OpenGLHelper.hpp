#include "display/opengl/OpenGL.hpp"

#ifndef OPENGL_HELPER_HPP_
#define OPENGL_HELPER_HPP_

#define FIXED(value) OpenGLHelper::floatToFixed(value)

//Class containing static OpenGL related helper methods
class OpenGLHelper
{
public:	
	static void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz);
	static void configureLightAndMaterial();
	static GLObject* newGLObject(long vertices, int vertexComponents, int textureComponents);
	static GLObject* CreateTexturedQuad(int textureWidth, int textureHeight, int size);
	static void drawGLObject(GLObject *object);
	static void freeGLObject(GLObject *object);
	static long floatToFixed(float value);
};

#endif