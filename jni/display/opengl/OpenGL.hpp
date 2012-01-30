#ifndef OPENGL_RENDERER_HPP
#define OPENGL_RENDERER_HPP

#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "AmplifyRealityGlobals.hpp"

#include "math.h"
#include "android_native_app_glue.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "display/opengl/GLObject.hpp"


#define USE_POWER2_TEXTURES true

#undef PI
#define PI 3.1415926535897932f

using namespace std;

class OpenGL
{
	
private:
	struct timespec start, end;
	EGLDisplay eglDisplay;
	EGLSurface eglSurface;
	EGLContext eglContext;

	void InitializeEGL(ANativeWindow * window);
	void InitializeShaders();
	void LinkProgram(GLuint program);
	void CompileShader(GLuint shaderId, const char * shaderCode);
	void Teardown();

public:
	int32_t screenWidth;
	int32_t screenHeight;
	OpenGL(ANativeWindow* window);
	void DrawGLObject(GLObject * object);
	void StartFrame();
	void EndFrame();
	
	void SetAttributeLocations();
	
	OpenGLRenderData renderData;
	GLuint uiProgramObject;      // shader program handle

	~OpenGL();


};

#endif
