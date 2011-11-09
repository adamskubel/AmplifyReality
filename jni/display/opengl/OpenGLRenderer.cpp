#include "OpenGLRenderer.hpp"

// Capped conversion from float to fixed.
static long floatToFixed(float value)
{
	if (value < -32768)
		value = -32768;
	if (value > 32767)
		value = 32767;
	return (long) (value * 65536);
}

#define FIXED(value) floatToFixed(value)

static OpenGLRenderer::GLOBJECT * groundPlane;

GLfixed render_count = 0;
GLuint textureID;

void OpenGLRenderer::render(int screenWidth, int screenHeight, int imageWidth, int imageHeight, void * pixels)
{
	// Prepare OpenGL ES for rendering of the frame.
	prepareFrame(screenWidth, screenHeight, imageWidth, imageHeight);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	drawGroundPlane();

	eglSwapBuffers(myEngine->display, myEngine->surface);
}

void OpenGLRenderer::freeGLObject(GLOBJECT *object)
{
	if (object == NULL)
		return;
	delete[] object->colorArray;
	delete[] object->vertexArray;
	delete object;
}

OpenGLRenderer::GLOBJECT * OpenGLRenderer::newGLObject(long vertices, int vertexComponents, int textureComponents)
{
	GLOBJECT * result = new GLOBJECT;

	result->count = vertices;
	result->vertexComponents = vertexComponents;
	result->textureComponents = textureComponents;

	result->vertexArray = new GLfixed[vertices * vertexComponents];
	result->textureArray = new GLfixed[vertices * textureComponents];
	result->colorArray = new GLubyte[vertices * 4];

	return result;
}

void OpenGLRenderer::drawGLObject(GLOBJECT *object)
{
	glVertexPointer(object->vertexComponents, GL_FIXED, 0, object->vertexArray);
	glTexCoordPointer(object->textureComponents, GL_FIXED, 0, object->textureArray);
	//glColorPointer(4, GL_UNSIGNED_BYTE, 0, object->colorArray);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, object->count);
}

OpenGLRenderer::GLOBJECT * OpenGLRenderer::createTexturedQuad()
{
	LOGD("Creating textured quad");
	GLOBJECT *result;
	result = newGLObject(4, 3, 2);

	result->textureArray[0] = floatToFixed(0);
	result->textureArray[1] = floatToFixed(1);
	result->textureArray[2] = floatToFixed(1);
	result->textureArray[3] = floatToFixed(1);
	result->textureArray[4] = floatToFixed(0);
	result->textureArray[5] = floatToFixed(0);
	result->textureArray[6] = floatToFixed(1);
	result->textureArray[7] = floatToFixed(0);

	GLfixed size = floatToFixed(10);

	result->vertexArray[0] = -size;
	result->vertexArray[1] = -size;
	result->vertexArray[2] = 0;
	result->vertexArray[3] = -size;
	result->vertexArray[4] = size;
	result->vertexArray[5] = 0;
	result->vertexArray[6] = size;
	result->vertexArray[7] = -size;
	result->vertexArray[8] = 0;
	result->vertexArray[9] = size;
	result->vertexArray[10] = size;
	result->vertexArray[11] = 0;

	LOGD("Image quad complete");
	return result;
}

void OpenGLRenderer::drawGroundPlane()
{
	//glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	//glDisable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D);
	drawGLObject(groundPlane);

//	glEnable(GL_LIGHTING);
//	glDisable(GL_BLEND);
//	glEnable(GL_DEPTH_TEST);
}

void OpenGLRenderer::initOpenGL(ANativeWindow* window, int imageWidth, int imageHeight)
{
	//importGLInit();

	myEngine = new engine;

	const EGLint attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE };
	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;
//	EGLSurface pBuffer;

	LOGI("Setting up display");
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);
	LOGI("Display initialized");
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(window, 0, 0, format);

	LOGI("Creating window surface");

	surface = eglCreateWindowSurface(display, config, window, NULL);

	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
	{
		LOGW("Unable to eglMakeCurrent");
	}

	LOGI("Getting surface params");
	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

//	EGLint list[] = {
//	// Specify the size of the surface.
//			EGL_WIDTH, w, EGL_HEIGHT, h,
//			// Target for the texture to store in the pbuffer.
//			EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
//			// The format of the texture that will be created when the pBuffer is bound to a texture.
//			EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
//			//Texture binding
//			EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
//			// Signal the end.
//			EGL_NONE };
//	pBuffer = eglCreatePbufferSurface(display, config, list);

	myEngine->display = display;
	myEngine->context = context;
	myEngine->surface = surface;
//	myEngine->pBuffer = pBuffer;
	myEngine->width = w;
	myEngine->height = h;

	LOGI("Setting OpenGL parameters");
//	glEnable(GL_NORMALIZE);
//	glEnable(GL_DEPTH_TEST);
//	glDisable(GL_CULL_FACE);
	glShadeModel(GL_FLAT);

//	glEnable(GL_LIGHTING);
//	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
//	glEnable(GL_LIGHT2);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	LOGI("Creating texture, width=%d, height=%d", imageWidth, imageHeight);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &textureID);
	LOGD("Texture ID is %d", textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); // new GLubyte[w * h]); //Create texture with blank data
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	groundPlane = createTexturedQuad(); // createGroundPlane();
}

void OpenGLRenderer::teardownOpenGL()
{



	if (myEngine->display != EGL_NO_DISPLAY)
	{
		eglMakeCurrent(myEngine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (myEngine->context != EGL_NO_CONTEXT)
		{
			eglDestroyContext(myEngine->display, myEngine->context);
		}
		if (myEngine->surface != EGL_NO_SURFACE)
		{
			eglDestroySurface(myEngine->display, myEngine->surface);
		}
//		if (myEngine->pBuffer != EGL_NO_SURFACE)
//		{
//			eglDestroySurface(myEngine->display, myEngine->pBuffer);
//		}
		eglTerminate(myEngine->display);
	}

	glDeleteTextures(1,&textureID);

	myEngine->display = EGL_NO_DISPLAY;
	myEngine->context = EGL_NO_CONTEXT;
	myEngine->surface = EGL_NO_SURFACE;
//	myEngine->pBuffer = EGL_NO_SURFACE;

	freeGLObject(groundPlane);
}

void OpenGLRenderer::prepareFrame(int screenWidth, int screenHeight, int imageWidth, int imageHeight)
{
	glViewport(0, 0, screenWidth, screenHeight);

	glClearColorx(0,0,0,0);//(GLfixed) (0.1f * 65536), (GLfixed) (0.2f * 65536), (GLfixed) (0.3f * 65536), 0x10000);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho(20, 20, -10, 10);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	GLfloat scaleFactorX = ((GLfloat) screenHeight) / ((GLfloat) imageHeight);
	GLfloat scaleFactorY = ((GLfloat) screenWidth) / ((GLfloat) imageWidth);
	GLfloat scale = max(scaleFactorX, scaleFactorY);
	GLfloat translateValue = (1.0f - scale) * 10.0f;
	glScalef(scale, -scale, 1);
	glRotatef(-90, 0, 0, 1);
	glTranslatef(translateValue, -translateValue, 0);
}

void OpenGLRenderer::gluOrtho(GLfloat height, GLfloat width, GLfloat zNear, GLfloat zFar)
{
	GLfloat xmin, xmax, ymin, ymax, aspect;

	aspect = height / width;

	ymax = height / 2;
	ymin = -ymax;
	xmin = -width / 2;
	xmax = -xmin;

	glOrthox((GLfixed) (xmin * 65536), (GLfixed) (xmax * 65536), (GLfixed) (ymin * 65536), (GLfixed) (ymax * 65536), (GLfixed) (zNear * 65536), (GLfixed) (zFar * 65536));
}

/* Following gluLookAt implementation is adapted from the
 * Mesa 3D Graphics library. http://www.mesa3d.org
 */
void OpenGLRenderer::gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz)
{
	GLfloat m[16];
	GLfloat x[3], y[3], z[3];
	GLfloat mag;

	/* Make rotation matrix */

	/* Z vector */
	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;
	mag = (float) sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
	if (mag)
	{ /* mpichler, 19950515 */
		z[0] /= mag;
		z[1] /= mag;
		z[2] /= mag;
	}

	/* Y vector */
	y[0] = upx;
	y[1] = upy;
	y[2] = upz;

	/* X vector = Y cross Z */
	x[0] = y[1] * z[2] - y[2] * z[1];
	x[1] = -y[0] * z[2] + y[2] * z[0];
	x[2] = y[0] * z[1] - y[1] * z[0];

	/* Recompute Y = Z cross X */
	y[0] = z[1] * x[2] - z[2] * x[1];
	y[1] = -z[0] * x[2] + z[2] * x[0];
	y[2] = z[0] * x[1] - z[1] * x[0];

	/* mpichler, 19950515 */
	/* cross product gives area of parallelogram, which is < 1.0 for
	 * non-perpendicular unit-length vectors; so normalize x, y here
	 */

	mag = (float) sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
	if (mag)
	{
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}

	mag = (float) sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
	if (mag)
	{
		y[0] /= mag;
		y[1] /= mag;
		y[2] /= mag;
	}

#define M(row,col)  m[col*4+row]
	M(0, 0) = x[0];
	M(0, 1) = x[1];
	M(0, 2) = x[2];
	M(0, 3) = 0.0;
	M(1, 0) = y[0];
	M(1, 1) = y[1];
	M(1, 2) = y[2];
	M(1, 3) = 0.0;
	M(2, 0) = z[0];
	M(2, 1) = z[1];
	M(2, 2) = z[2];
	M(2, 3) = 0.0;
	M(3, 0) = 0.0;
	M(3, 1) = 0.0;
	M(3, 2) = 0.0;
	M(3, 3) = 1.0;
#undef M
	{
		int a;
		GLfixed fixedM[16];
		for (a = 0; a < 16; ++a)
			fixedM[a] = (GLfixed) (m[a] * 65536);
		glMultMatrixx(fixedM);
	}

	/* Translate Eye to Origin */
	glTranslatex((GLfixed) (-eyex * 65536), (GLfixed) (-eyey * 65536), (GLfixed) (-eyez * 65536));
}

