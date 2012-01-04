#include "OpenGLRenderer.hpp"

// Capped conversion from float to fixed.
static long floatToFixed(float value) {
	if (value < -32768)
		value = -32768;
	if (value > 32767)
		value = 32767;
	return (long) (value * 65536);
}

#define FIXED(value) floatToFixed(value)

static OpenGLRenderer::GLOBJECT * texturedQuad;



static int bgSize = 20; 

void OpenGLRenderer::render(int imageWidth, int imageHeight, void * pixels) {
	// Prepare OpenGL ES for rendering of the frame.
	prepareFrame(imageWidth, imageHeight);

	glBindTexture(GL_TEXTURE_2D, myEngine->textureID);
	
	//Debugging - Draw a solid color to texture
	if (ENABLE_TEXTURE_COLOR)
	{
		u_int32_t * pxData = new u_int32_t[myEngine->textureWidth*myEngine->textureHeight];
		for (int i=0;i<(myEngine->textureWidth*myEngine->textureHeight);i++)
		{
			pxData[i] = 123912048;
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, myEngine->textureWidth, myEngine->textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, pxData);
		delete[] pxData;
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	
	drawTexturedObject(texturedQuad);

	eglSwapBuffers(myEngine->display, myEngine->surface);
}

void OpenGLRenderer::calculateTextureSize(int imageWidth, int imageHeight, int * textureWidth, int * textureHeight)
{
	if (USE_POWER2_TEXTURES)
	{
		int heightLog = ceilf(logbf((float)imageHeight))+1;
		int widthLog = ceilf(logbf((float)imageWidth))+1;

		*textureHeight = (int)pow(2,heightLog);
		*textureWidth = (int)pow(2,widthLog);
	}
	else
	{
		*textureWidth = imageWidth;
		*textureHeight = imageHeight;
	}
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
	glTexCoordPointer(object->textureComponents, GL_FIXED, 0,object->textureArray);
	//glColorPointer(4, GL_UNSIGNED_BYTE, 0, object->colorArray);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, object->count);
}

OpenGLRenderer::GLOBJECT * OpenGLRenderer::createTexturedQuad(int textureWidth, int textureHeight, int size) 
{

	float aspectRatio = ((float)textureWidth)/textureHeight;

	GLfloat yLimit;
	GLfloat xLimit;
	if (aspectRatio > 1)
	{
		xLimit = size;
		yLimit = ((float)size) / aspectRatio;
	}
	else
	{
		xLimit = ((float)size) * aspectRatio;
		yLimit = size;
	}

	LOGD("OpenGL","Creating textured quad");
	GLOBJECT *result;
	result = newGLObject(4, 3, 2);

	result->textureArray[0] = floatToFixed(0);
	result->textureArray[1] = floatToFixed(0);

	result->textureArray[2] = floatToFixed(0);
	result->textureArray[3] = floatToFixed(1);

	result->textureArray[4] = floatToFixed(1);
	result->textureArray[5] = floatToFixed(0);

	result->textureArray[6] = floatToFixed(1);
	result->textureArray[7] = floatToFixed(1);


	result->vertexArray[0] = 0; //x
	result->vertexArray[1] = 0; //y
	result->vertexArray[2] = 0; //z

	result->vertexArray[3] = 0;
	result->vertexArray[4] = floatToFixed(yLimit);
	result->vertexArray[5] = 0;

	result->vertexArray[6] = floatToFixed(xLimit);
	result->vertexArray[7] = 0;
	result->vertexArray[8] = 0;

	result->vertexArray[9] = floatToFixed(xLimit);
	result->vertexArray[10] = floatToFixed(yLimit);
	result->vertexArray[11] = 0;

	result->width = xLimit;
	result->height = yLimit;

	LOGD("OpenGL","Image quad complete");
	return result;
}

void OpenGLRenderer::drawTexturedObject(GLOBJECT * glObject) 
{
	//glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	//glDisable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D);
	drawGLObject(glObject);

//	glEnable(GL_LIGHTING);
//	glDisable(GL_BLEND);
//	glEnable(GL_DEPTH_TEST);
}


static float countF = -20.0f;

void OpenGLRenderer::initOpenGL(ANativeWindow* window, int imageWidth, int imageHeight) 
{
	myEngine = new engine;
	const EGLint attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE,
			8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE };

	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	LOGI("OpenGL","Setting up display");
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);
	LOGI("OpenGL","Display initialized");
	
	
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(window, 0, 0, format);

	LOGI("OpenGL","Creating window surface");

	surface = eglCreateWindowSurface(display, config, window, NULL);

	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("OpenGL","Unable to eglMakeCurrent");
	}

	LOGI("OpenGL","Getting surface params");
	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);
	LOGI("OpenGL","Surface parameters: width=%d, height=%d", w, h);

	myEngine->display = display;
	myEngine->context = context;
	myEngine->surface = surface;
	myEngine->screenWidth = w;
	myEngine->screenHeight = h;




	LOGI("OpenGL","Setting OpenGL parameters");
	glShadeModel(GL_FLAT);

//	glEnable(GL_LIGHTING);
//	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
//	glEnable(GL_LIGHT2);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//Create the texture object
	calculateTextureSize(imageWidth,imageHeight,&myEngine->textureWidth,&myEngine->textureHeight);
	LOGI("OpenGL","Creating texture, width=%d, height=%d", myEngine->textureWidth, myEngine->textureHeight);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &(myEngine->textureID));
	LOGD("OpenGL","Texture ID is %d", (myEngine->textureID));
	glBindTexture(GL_TEXTURE_2D, (myEngine->textureID));

	//Initialize texture with blank data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, myEngine->textureWidth, myEngine->textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	texturedQuad = createTexturedQuad(myEngine->textureWidth,myEngine->textureHeight,bgSize); 

	glViewport(0, 0, myEngine->screenWidth, myEngine->screenHeight);

}

void OpenGLRenderer::teardownOpenGL() 
{
	if (myEngine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(myEngine->display, EGL_NO_SURFACE, EGL_NO_SURFACE,
				EGL_NO_CONTEXT);
		if (myEngine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(myEngine->display, myEngine->context);
		}
		if (myEngine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(myEngine->display, myEngine->surface);
		}
		eglTerminate(myEngine->display);
	}
	glDeleteTextures(1, &myEngine->textureID);

	myEngine->display = EGL_NO_DISPLAY;
	myEngine->context = EGL_NO_CONTEXT;
	myEngine->surface = EGL_NO_SURFACE;
	freeGLObject(texturedQuad);
}

void OpenGLRenderer::prepareFrame(int imageWidth, int imageHeight)
{
	glClearColorx(0, 0, 0, 0); 
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//Define projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	

	float aspectRatio = ((float)myEngine->screenWidth)/myEngine->screenHeight;

	GLfloat orthoHeight;
	GLfloat orthoWidth;
	if (aspectRatio > 1)
	{
		orthoWidth = aspectRatio * bgSize;
		orthoHeight = bgSize;
	}
	else
	{
		orthoWidth = bgSize;
		orthoHeight = ((float)bgSize) / aspectRatio;
	}
	
	glOrthof(0,orthoWidth,0,orthoHeight,-10,10);

	//Define model matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Assume input image is mirrored on Y axis
	//Assume X-Y scale is the same - choose Y
	//Ymax in device coords is orthoHeight
	//Yimg in device coords is imageHeight/textureHeight * bgSize 
	//Scale= -Ymax/Yimg

	float yImg = ((float)imageHeight/(float)myEngine->textureHeight)* texturedQuad->height;
	float yScale = -(float)orthoHeight/(float)yImg;
	
	float xImg = ((float)imageWidth/(float)myEngine->textureWidth)* texturedQuad->width;
	float xScale = (float)orthoWidth/(float)xImg;

	LOGD("OpenGL","xScale=%f,yScale=%f",xScale,yScale);
	
	glScalef(-yScale,yScale,1);
	
	glTranslatef(0,-yImg,0);

}


/* Following gluLookAt implementation is adapted from the
 * Mesa 3D Graphics library. http://www.mesa3d.org
 */
void OpenGLRenderer::gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
		GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx,
		GLfloat upy, GLfloat upz) {
	GLfloat m[16];
	GLfloat x[3], y[3], z[3];
	GLfloat mag;

	/* Make rotation matrix */

	/* Z vector */
	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;
	mag = (float) sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
	if (mag) { /* mpichler, 19950515 */
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
	if (mag) {
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}

	mag = (float) sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
	if (mag) {
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
			fixedM[a] = (GLfixed)(m[a] * 65536);
		glMultMatrixx(fixedM);
	}

	/* Translate Eye to Origin */
	glTranslatex((GLfixed)(-eyex * 65536), (GLfixed)(-eyey * 65536),
			(GLfixed)(-eyez * 65536));
}

