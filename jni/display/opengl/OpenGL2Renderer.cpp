#include "OpenGL.hpp"


OpenGL::OpenGL(ANativeWindow * window)
{
	InitializeEGL(window);
	
	InitializeShaders();
}

OpenGL::~OpenGL()
{
	Teardown();
}


void OpenGL::Teardown()
{
	if (eglDisplay != EGL_NO_DISPLAY) 
	{
		eglMakeCurrent( eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if ( eglContext != EGL_NO_CONTEXT) 
		{
			eglDestroyContext(eglDisplay, eglContext);
		}
		if ( eglSurface != EGL_NO_SURFACE) 
		{
			eglDestroySurface(eglDisplay, eglSurface);
		}
		eglTerminate(eglDisplay);
	}

	eglDisplay = EGL_NO_DISPLAY;
	eglContext = EGL_NO_CONTEXT;
	eglSurface = EGL_NO_SURFACE;
}

void OpenGL::InitializeEGL(ANativeWindow* androidWindow)
{
	eglDisplay    = EGL_NO_DISPLAY; 
	eglSurface    = EGL_NO_SURFACE; 
	eglContext    = EGL_NO_CONTEXT; 
	
	EGLConfig eglConfig = 0; 
	EGLNativeDisplayType eglNativeDisplay = EGL_DEFAULT_DISPLAY; 
	EGLint iErr = 0; 


	eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	EGLint iMajorVersion, iMinorVersion; 
	if (!eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion)) 
	{ 
		LOGE("Error: eglInitialize() failed."); 
		return;
	}

	LOGI(LOGTAG_OPENGL,"Initialized EGL. Version=%d:%d",iMajorVersion,iMinorVersion);


	if (!eglBindAPI(EGL_OPENGL_ES_API))
	{
		LOGE("Failed to bind EGL to OpenGL ES Api");
		return;
	}
	LOGI(LOGTAG_OPENGL,"Bound to API");
		
	const EGLint attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,EGL_BLUE_SIZE,
		8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 4, EGL_NONE };

	int iConfigs; 
	if(!eglChooseConfig(eglDisplay, attribs, &eglConfig, 1, &iConfigs) ||  
		(iConfigs != 1)) 
	{ 
		LOGE("Error: eglChooseConfig() failed."); 
		return;
	} 
	LOGI(LOGTAG_OPENGL,"EGL Configuration successful");
	
	EGLint format;
	eglGetConfigAttrib(eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(androidWindow, 0, 0, format);

	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, androidWindow, NULL); 
	if((iErr = eglGetError()) != EGL_SUCCESS) 
	{ 
		LOGE("eglCreateWindowSurface failed (%d)", iErr); 
		return;
	}
	LOGI(LOGTAG_OPENGL,"EGL window surface created.");

	EGLint pi32ContextAttribs[3]; 
	pi32ContextAttribs[0] = EGL_CONTEXT_CLIENT_VERSION; 
	pi32ContextAttribs[1] = 2; 
	pi32ContextAttribs[2] = EGL_NONE; 

	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, pi32ContextAttribs); 

	if((iErr = eglGetError()) != EGL_SUCCESS)
	{
		LOGE("Failed to create EGL Context");
	}
	LOGI(LOGTAG_OPENGL,"EGL context created successfully");

	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext); 

	if((iErr = eglGetError()) != EGL_SUCCESS) 
	{
		LOGE("Failed to make EGL context current");
		return;
	}
	LOGI(LOGTAG_OPENGL,"EGL context is now current");

	eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &screenWidth);
	eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &screenHeight);
	int depthBits[1] = {0};
	glGetIntegerv(GL_DEPTH_BITS, depthBits);
	LOGI(LOGTAG_OPENGL,"Surface parameters: width=%d, height=%d, Depth Bits = %d", screenWidth,screenHeight,depthBits[0]);


}

void OpenGL::StartFrame()
{	
	glViewport(0,0,screenWidth,screenHeight);
	glClearColor(0,0,0.4f,1);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	
}

void OpenGL::DrawGLObject(GLObject *object) 
{
	object->Draw(renderData);
}

void OpenGL::EndFrame()
{		
	eglSwapBuffers( eglDisplay, eglSurface);
}


void OpenGL::InitializeShaders()
{
	LOGI(LOGTAG_OPENGL,"Initializing shaders");	
	const char * g_strVSProgram =
		"\
		attribute vec3 Position;\
		attribute vec4 SourceColor;\
		varying   vec4 DestinationColor;\
		\
		uniform mat4 Projection;\
		uniform mat4 ModelView;\
		\
		attribute vec2 TexCoordIn;\
		varying vec2 TexCoordOut;\
		\
		void main()\
		{\
			gl_Position  = Projection * ModelView * vec4(Position, 1.0);\
			DestinationColor = SourceColor;\
			TexCoordOut=TexCoordIn;\
		}\
		";

	const char * g_strFSProgram =
		"\
		varying lowp vec4 DestinationColor;\
		\
		varying lowp vec2 TexCoordOut;\
		uniform sampler2D Texture;\
		uniform bool useTexture;\
		\
		void main()\
		{\
			if (useTexture)\
				gl_FragColor = texture2D(Texture, TexCoordOut);\
			else\
				gl_FragColor = DestinationColor;\
		}\
		";	


	//const char * lightShader =
	//	"\
	//	varying vec3 normal;\
	//	attribute vec3 LightDir, LightPos;\
	//	attribute vec4 AmbientColor;\
	//	attribute vec4 DiffuseColor;\
	//	attribute vec4 SpecularColor;\
	//	\
	//	void main (void)\
	//	{\
	//		vec4 final_color = AmbientColor;\
	//		vec3 N = normalize(normal);\
	//		vec3 L = normalize(lightDir);\
	//		\
	//		float lambertTerm = dot(N,L);\
	//		\
	//		if(lambertTerm > 0.0)\
	//		{\
	//			final_color += DiffuseColor * lambertTerm;\
	//			vec3 E = normalize(lightPos);\
	//			vec3 R = reflect(-L, N);\
	//			float specular = pow( max(dot(R, E), 0.0), 0.5f);\
	//			final_color += SpecularColor * specular;\
	//		}\
	//		gl_FragColor = final_color;\
	//	}\
	//	";

	
	//const char * lightShader_vertex = 
	//	"void main()\
	//		{\
	//		vec3 normal, lightDir;\
	//		vec4 diffuse, ambient, globalAmbient;\
	//		float NdotL;\
	//		\
	//		normal = normalize(gl_NormalMatrix * gl_Normal);\
	//		lightDir = normalize(vec3(gl_LightSource[0].position));\
	//		NdotL = max(dot(normal, lightDir), 0.0);\
	//		diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;\
	//		\
	//		/* Compute the ambient and globalAmbient terms */\
	//		ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;\
	//		globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;\
	//		\
	//		gl_FrontColor =  NdotL * diffuse + globalAmbient + ambient;\
	//		\
	//		gl_Position = ftransform();\
	//	}\
	//	";

	//const char * lightShader_frag = 
	//"varying vec4 diffuse,ambientGlobal, ambient;\
	//varying vec3 normal,lightDir,halfVector;\
	//varying float dist;\
	//\
	//\
	//void main()\
	//{\
	//	vec3 n,halfV,viewV,ldir;\
	//	float NdotL,NdotHV;\
	//	vec4 color = ambientGlobal;\
	//	float att;\
	//	\
	//	/* a fragment shader can't write a varying variable, hence we need\
	//	a new variable to store the normalized interpolated normal */\
	//	n = normalize(normal);\
	//	\
	//	/* compute the dot product between normal and normalized lightdir */\
	//	NdotL = max(dot(n,normalize(lightDir)),0.0);\
	//\
	//	if (NdotL > 0.0) {\
	//	\
	//		att = 1.0 / (gl_LightSource[0].constantAttenuation +\
	//				gl_LightSource[0].linearAttenuation * dist +\
	//				gl_LightSource[0].quadraticAttenuation * dist * dist);\
	//		color += att * (diffuse * NdotL + ambient);\
	//	\
	//		\
	//		halfV = normalize(halfVector);\
	//		NdotHV = max(dot(n,halfV),0.0);\
	//		color += att * gl_FrontMaterial.specular * gl_LightSource[0].specular * \
	//						pow(NdotHV,gl_FrontMaterial.shininess);\
	//	}\
	//\
	//	gl_FragColor = color;\
	//}\
	//";

	// Create the fragment shader object 
	LOGD(LOGTAG_OPENGL,"Creating fragment shader");
	renderData.fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); 	
	LOGD(LOGTAG_OPENGL,"Complete. Compiling..");
	CompileShader(renderData.fragmentShader,g_strFSProgram);
	LOGD(LOGTAG_OPENGL,"Complete. Creating vertex shader.");

	// Loads the vertex shader in the same way 
	renderData.vertexShader = glCreateShader(GL_VERTEX_SHADER); 
	LOGD(LOGTAG_OPENGL,"Complete. Compiling..");
	CompileShader(renderData.vertexShader,g_strVSProgram);
	LOGD(LOGTAG_OPENGL,"Complete. Creating program.");

	// Create the shader program 
	uiProgramObject = glCreateProgram(); 
	
	LOGD(LOGTAG_OPENGL,"Complete. Attaching shaders...");
	// Attach the fragment and vertex shaders to it 
	glAttachShader(uiProgramObject, renderData.fragmentShader); 
	glAttachShader(uiProgramObject, renderData.vertexShader); 
	
	LOGD(LOGTAG_OPENGL,"Complete. Linking program...");

	// Link the program 
	LinkProgram(uiProgramObject);
	
	LOGD(LOGTAG_OPENGL,"Complete. Activating program...");

	//Activate shader program
	glUseProgram(uiProgramObject); 

	LOGD(LOGTAG_OPENGL,"Complete. Setting addresses...");

	
	//Set variable locations
	SetAttributeLocations();

	
	LOGD(LOGTAG_OPENGL,"Complete. Enabling arrays...");
	//Enable arrays
	glEnableVertexAttribArray(renderData.vertexArrayLocation);
	glEnableVertexAttribArray(renderData.textureArrayLocation);
	glEnableVertexAttribArray(renderData.colorArrayLocation);
	
	LOGI(LOGTAG_OPENGL,"ProjectLocation=%d,ModelMatLocation=%d,VertexLocation=%d,ColorLocation=%d",
		renderData.projectionMatrixLocation,renderData.modelMatrixLocation,renderData.vertexArrayLocation,
		renderData.colorArrayLocation);
	LOGI(LOGTAG_OPENGL,"Complete. That's it, so...shader initialization complete!");

}

void OpenGL::SetAttributeLocations()
{	
	renderData.vertexArrayLocation = glGetAttribLocation(uiProgramObject,"Position");
	renderData.textureArrayLocation = glGetAttribLocation(uiProgramObject,"TexCoordIn");
	renderData.colorArrayLocation =	glGetAttribLocation(uiProgramObject,"SourceColor");

	/*renderData.diffuseColorLocation =	glGetAttribLocation(uiProgramObject,"DiffuseColor");
	renderData.ambientColorLocation =	glGetAttribLocation(uiProgramObject,"AmbientColor");
	renderData.specularColorLocation =	glGetAttribLocation(uiProgramObject,"SpecularColor");*/

	renderData.modelMatrixLocation=  glGetUniformLocation(uiProgramObject,"ModelView");
	renderData.projectionMatrixLocation = glGetUniformLocation(uiProgramObject,"Projection");
	renderData.textureLocation = glGetUniformLocation(uiProgramObject, "Texture");
	renderData.useTextureFlagLocation = glGetUniformLocation(uiProgramObject, "useTexture");
}

//Link, then do error checking
void OpenGL::LinkProgram(GLuint programObject)
{	
	glLinkProgram(programObject); 

	// Check if linking succeeded in the same way we checked for compilation success 
	GLint bLinked; 
	glGetProgramiv(programObject, GL_LINK_STATUS, &bLinked); 

	if (!bLinked) 
	{ 
		int ui32InfoLogLength, ui32CharsWritten; 
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &ui32InfoLogLength); 
		char* pszInfoLog = new char[ui32InfoLogLength]; 
		glGetProgramInfoLog(programObject, ui32InfoLogLength, &ui32CharsWritten, 
			pszInfoLog); 
		LOGE("Failed to link program: %s", pszInfoLog); 
		delete [] pszInfoLog; 
		throw OpenGLInitializationException();
	} 
}

//Compile, then do error checking
void OpenGL::CompileShader(GLuint shader, const char * shaderSource)
{	
	GLint bShaderCompiled; 
	glShaderSource(shader, 1, &shaderSource, NULL); 
	glCompileShader(shader); 
	glGetShaderiv(shader, GL_COMPILE_STATUS, &bShaderCompiled); 

	if (!bShaderCompiled) 
	{ 
		int i32InfoLogLength, i32CharsWritten; 
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &i32InfoLogLength); 
		char * infoLog = new char[i32InfoLogLength];
		glGetShaderInfoLog(shader, i32InfoLogLength, &i32CharsWritten, infoLog); 
		LOGE("Failed to compile shader(log length = %d): %s",i32InfoLogLength, infoLog); 
		delete [] infoLog; 
		throw OpenGLInitializationException();
	} 
	LOGI(LOGTAG_OPENGL,"Compiled shader successfully");
}