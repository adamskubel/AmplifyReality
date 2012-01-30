#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "OpenGLRenderData.hpp"

#ifndef GLOBJECT_HPP_
#define GLOBJECT_HPP_


class GLObject
{
public:
	static const GLint vertexComponents = 3; //Always has 3 vertex componenents (XYZ)

	virtual ~GLObject()
	{
		;//Base destructor
	}
	virtual void Draw(OpenGLRenderData renderData);

	GLfloat *vertexArray;
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

		vertexArray = new GLfloat[count *  GLObject::vertexComponents];
		textureArray = new GLfloat[count * textureComponents];
		colorArray = new GLfloat[count * 4];

		for (int i=0;i<count*4;i++)
		{
			colorArray[i] = 1.0f;
		}

	}
	~TexturedGLObject()
	{
		delete[] textureArray;
		delete[] vertexArray;
		delete[] colorArray;
	}

	void Draw(OpenGLRenderData renderData)
	{

		glUniform1i(renderData.useTextureFlagLocation,1);

		glEnableVertexAttribArray(renderData.vertexArrayLocation);
		glEnableVertexAttribArray(renderData.textureArrayLocation);
		glEnableVertexAttribArray(renderData.colorArrayLocation);

		glVertexAttribPointer( renderData.vertexArrayLocation,GLObject::vertexComponents, GL_FLOAT, 0, 0, vertexArray );		
		glVertexAttribPointer( renderData.colorArrayLocation, 4, GL_FLOAT, 0, 0, colorArray);
		glVertexAttribPointer( renderData.textureArrayLocation, textureComponents , GL_FLOAT, 0, 0, textureArray);
			
		glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
	}
	
//	GLuint bufferID;
	GLfloat *colorArray;
	GLfloat *textureArray;
	GLint textureComponents;
};

class ColorGLObject : public GLObject
{

public:
	static const GLint colorComponents = 4; //Always has 4 color components (RGBA)
	
	//GLfloat *textureArray;
	GLfloat *colorArray;
	int renderGroupSize;

	ColorGLObject(int _vertices, int _renderGroupSize = 1)
	{			
		renderGroupSize = _renderGroupSize;
		count = _vertices;

		vertexArray = new GLfloat[count * GLObject::vertexComponents];
		colorArray = new GLfloat[count * ColorGLObject::colorComponents];		
		//textureArray = new GLfloat[count * 2];

		//for (int i=0;i<count*2;i++)
		//{
		//	textureArray[i] = -1.0f;
		//}
	}	
	~ColorGLObject()
	{
		delete[] colorArray;
		delete[] vertexArray;
	//	delete[] textureArray;
	}

	void Draw(OpenGLRenderData renderData)
	{		
		
		glUniform1i(renderData.useTextureFlagLocation,0);

		glEnableVertexAttribArray( renderData.vertexArrayLocation );
		glEnableVertexAttribArray( renderData.colorArrayLocation );

		glVertexAttribPointer( renderData.vertexArrayLocation, GLObject::vertexComponents, GL_FLOAT, 0, 0, vertexArray );
		glVertexAttribPointer( renderData.colorArrayLocation, ColorGLObject::colorComponents, GL_FLOAT, 0, 0, colorArray);
		
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
	
};
#endif
