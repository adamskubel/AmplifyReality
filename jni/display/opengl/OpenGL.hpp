#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "AmplifyRealityGlobals.hpp"

#include "math.h"
#include "android_native_app_glue.h"

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#include "display/opengl/GLObject.hpp"


#ifndef OPENGL_RENDERER_HPP
#define OPENGL_RENDERER_HPP

#define USE_POWER2_TEXTURES true

#undef PI
#define PI 3.1415926535897932f

using namespace std;



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
	void StartFrame();
	void EndFrame();
	~OpenGL();

};

#endif
