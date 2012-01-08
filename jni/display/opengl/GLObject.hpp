#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#ifndef GLOBJECT_HPP_
#define GLOBJECT_HPP_

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
#endif