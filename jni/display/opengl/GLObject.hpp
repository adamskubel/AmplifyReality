#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#ifndef GLOBJECT_HPP_
#define GLOBJECT_HPP_

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

class GLObject
{
public:
	static const GLint vertexComponents = 3; //Always has 3 vertex componenents (XYZ)

	virtual ~GLObject()
	{
		;//Base destructor
	}
	virtual void Draw();

	GLfixed *vertexArray;
	GLsizei count;
	GLfloat width,height;
};

class TexturedGLObject : public GLObject
{
	

public:
	TexturedGLObject(int _vertices, int _textureComponents)
	{
		count = _vertices;
		textureComponents = _textureComponents;

		vertexArray = new GLfixed[count *  GLObject::vertexComponents];
		textureArray = new GLfixed[count * textureComponents];
	}
	~TexturedGLObject()
	{
		delete[] textureArray;
		delete[] vertexArray;
	}

	void Draw()
	{
		glVertexPointer( GLObject::vertexComponents, GL_FIXED, 0, vertexArray);
		glTexCoordPointer(textureComponents, GL_FIXED, 0,textureArray);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
	}

	GLfixed *textureArray;
	GLint textureComponents;
};

class ColorGLObject : public GLObject
{

public:
	static const GLint colorComponents = 4; //Always has 4 color components (RGBA)

	int renderGroupSize;

	ColorGLObject(int _vertices, int _renderGroupSize = 1)
	{			
		renderGroupSize = _renderGroupSize;
		count = _vertices;


		vertexArray = new GLfixed[count * GLObject::vertexComponents];
		colorArray = new GLubyte[count * ColorGLObject::colorComponents];
	}	
	~ColorGLObject()
	{
		delete[] colorArray;
		delete[] vertexArray;
	}

	void Draw()
	{
		glVertexPointer( GLObject::vertexComponents, GL_FIXED, 0, vertexArray);
		glColorPointer(ColorGLObject::colorComponents, GL_UNSIGNED_BYTE, 0, colorArray);
		if (renderGroupSize > 1)
		{
			for (int i=0;i<count;i+=renderGroupSize)
			{
				glDrawArrays(GL_TRIANGLE_STRIP, i, renderGroupSize);
			}
		}
		else
			glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
	}

	
	GLubyte *colorArray;
};
#endif