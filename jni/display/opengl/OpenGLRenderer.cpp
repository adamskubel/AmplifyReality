#include "display/opengl/OpenGL.hpp"


OpenGL::OpenGL(ANativeWindow* window) 
{
	LOGI(LOGTAG_OPENGL,"Initializing OpenGL");
	const EGLint attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE,
		8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE };

	EGLint dummy, format;
	EGLint numConfigs;
	EGLConfig config;


	LOGI("OpenGL","Setting up display");
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);
	LOGI("OpenGL","Display initialized");


	eglChooseConfig(display, attribs, &config, 1, &numConfigs);
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(window, 0, 0, format);

	LOGI(LOGTAG_OPENGL,"Creating window surface");

	surface = eglCreateWindowSurface(display, config, window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) 
	{
		LOGE("OpenGL","Unable to eglMakeCurrent");
		throw new std::exception();		
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &screenWidth);
	eglQuerySurface(display, surface, EGL_HEIGHT, &screenHeight);
	LOGI(LOGTAG_OPENGL,"Surface parameters: width=%d, height=%d", screenWidth,screenHeight);
	
	LOGI(LOGTAG_OPENGL,"Setting OpenGL parameters");
	glShadeModel(GL_FLAT);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	glViewport(0, 0,  screenWidth,  screenHeight);
	LOGI(LOGTAG_OPENGL,"OpenGL initialization complete");		
}

void OpenGL::StartDraw()
{		
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OpenGL::DrawGLObject(GLObject *object) 
{
	object->Draw();
}

void OpenGL::Present()
{	
    glFlush();
	eglSwapBuffers( display,  surface);
}



//void OpenGLRenderer::drawTexturedObject(GLObject * GLObject) 
//{
//	//glDisable(GL_CULL_FACE);
//	//glDisable(GL_DEPTH_TEST);
//	//glEnable(GL_BLEND);
//	//glBlendFunc(GL_ZERO, GL_SRC_COLOR);
//	//glDisable(GL_LIGHTING);
//
//
//
////	glEnable(GL_LIGHTING);
////	glDisable(GL_BLEND);
////	glEnable(GL_DEPTH_TEST);
//}


OpenGL::~OpenGL() 
{
	if ( display != EGL_NO_DISPLAY) 
	{
		eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if ( context != EGL_NO_CONTEXT) 
		{
			eglDestroyContext( display,  context);
		}
		if ( surface != EGL_NO_SURFACE) 
		{
			eglDestroySurface( display,  surface);
		}
		eglTerminate( display);
	}

	display = EGL_NO_DISPLAY;
	context = EGL_NO_CONTEXT;
	surface = EGL_NO_SURFACE;
}



