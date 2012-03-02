#ifndef OPENGL_RENDER_DATA_HPP_
#define OPENGL_RENDER_DATA_HPP_

#include <EGL/egl.h>
#include <GLES2/gl2.h>
struct OpenGLRenderData
{
public:
	GLuint vertexArrayLocation;
	GLuint colorArrayLocation;
	GLuint normalArrayLocation;
	GLuint textureArrayLocation;

	GLuint projectionMatrixLocation;
	GLuint modelMatrixLocation;

	GLuint fragmentShader;
	GLuint vertexShader;

	GLuint textureLocation;

	GLuint useTextureFlagLocation;

	GLuint ambientColorLocation, specularColorLocation, diffuseColorLocation;
};

#endif