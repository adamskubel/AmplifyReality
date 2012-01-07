//#include "importgl.h"
#include "math.h"
#include <LogDefinitions.h>
#include "ExceptionCodes.hpp"

#include "android_native_app_glue.h"
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <DebugSettings.hpp>


#ifndef OPENGL_RENDERER_HPP
#define OPENGL_RENDERER_HPP

#define USE_POWER2_TEXTURES true

#undef PI
#define PI 3.1415926535897932f

using namespace std;

class GLObject
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

public:
	GLfixed *vertexArray;
	GLfixed *textureArray;
	GLubyte *colorArray;
	GLint vertexComponents;
	GLint textureComponents;
	GLsizei count;
	GLfloat width,height;	
};

class OpenGL
{
	
private:
	struct timespec start, end;
	EGLDisplay display;
	EGLSurface surface;
	EGLSurface pBuffer;
	EGLContext context;

public:
	int32_t screenWidth;
	int32_t screenHeight;
	OpenGL(ANativeWindow* window);
	void DrawGLObject(GLObject * object);
	~OpenGL();

};

#endif
