//#include "importgl.h"
#include "math.h"
#include <LogDefinitions.h>
#include "android_native_app_glue.h"
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <DebugSettings.hpp>


#ifndef OPENGL_RENDERER_HPP
#define OPENGL_RENDERER_HPP

#undef LOG_TAG
#define LOG_TAG "OpenGL"


#define USE_POWER2_TEXTURES true

#undef PI
#define PI 3.1415926535897932f

using namespace std;


struct OpenGLRenderer
{

	struct engine
	{
		EGLDisplay display;
		EGLSurface surface;
		EGLSurface pBuffer;
		EGLContext context;
		int32_t screenWidth;
		int32_t screenHeight;
		int32_t textureWidth;
		int32_t textureHeight;
		GLuint textureID;
	};

	struct GLOBJECT
	{
		/* Vertex array and color array are enabled for all objects, so their
		 * pointers must always be valid and non-NULL. Normal array is not
		 * used by the ground plane, so when its pointer is NULL then normal
		 * array usage is disabled.
		 *
		 * Vertex array is supposed to use GL_FIXED datatype and stride 0
		 * (i.e. tightly packed array). Color array is supposed to have 4
		 * components per color with GL_UNSIGNED_BYTE datatype and stride 0.
		 * Normal array is supposed to use GL_FIXED datatype and stride 0.
		 */
		GLfixed *vertexArray;
		GLfixed *textureArray;
		GLubyte *colorArray;
		GLint vertexComponents;
		GLint textureComponents;
		GLsizei count;

		GLfloat width,height;
	};
private:
	struct timespec start, end;
	engine * myEngine;

	void prepareFrame (int imageWidth, int imageHeight);

	static void calculateTextureSize(int imageWidth, int imageHeight, int * textureWidth, int * textureHeight);
	static void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz);
	static void configureLightAndMaterial();
	static GLOBJECT* createTexturedQuad(int textureWidth, int textureHeight, int size);
	static GLOBJECT* newGLObject(long vertices, int vertexComponents, int textureComponents);
	static void drawGLObject(GLOBJECT *object);
	static void drawTexturedObject(GLOBJECT *object);
	static void freeGLObject(GLOBJECT *object);

public:
	void render( int imageWidth, int imageHeight, void * pixels);
	void initOpenGL(ANativeWindow* window, int imageWidth, int imageHeight);
	void teardownOpenGL();

};

#endif
