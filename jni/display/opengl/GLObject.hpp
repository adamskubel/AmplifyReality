#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "OpenGLRenderData.hpp"
#include <opencv2/core/core.hpp>
#include "display/objloader/objLoader.h"
#include "display/Colors.hpp"


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
	GLsizei numVertices;
	GLfloat width,height;
};

class TexturedGLObject : public GLObject
{
	

public:
	TexturedGLObject(int _vertices, int _textureComponents)
	{
		numVertices = _vertices;
		textureComponents = _textureComponents;

		vertexArray = new GLfloat[numVertices *  GLObject::vertexComponents];
		textureArray = new GLfloat[numVertices * textureComponents];
		colorArray = new GLfloat[numVertices * 4];

		for (int i=0;i<numVertices*4;i++)
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
			
		glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
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
		numVertices = _vertices;

		vertexArray = new GLfloat[numVertices * GLObject::vertexComponents];
		colorArray = new GLfloat[numVertices * ColorGLObject::colorComponents];		
	}	
	~ColorGLObject()
	{
		delete[] colorArray;
		delete[] vertexArray;
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
			for (int i=0;i<numVertices;i+=renderGroupSize)
			{
				glDrawArrays(GL_TRIANGLE_STRIP, i, renderGroupSize);
			}
		}
		else
			glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);

	}
	
};

class WavefrontGLObject : public GLObject
{
	GLfloat *colorArray;
	GLfloat *colorArray_black;
	GLfloat *textureArray;
	GLubyte *indexArray;
	GLfloat *normalArray;
	
	bool hasTexture;

	int numFaces;
	int numNormals;

public:

	WavefrontGLObject(int _numVertices, int _numFaces)
	{			
		numVertices = _numVertices;
		numFaces = _numFaces;

		vertexArray = new GLfloat[numVertices * 3];
		colorArray = new GLfloat[numVertices * 4];	
		colorArray_black = new GLfloat[numVertices * 4];		
		textureArray = new GLfloat[numVertices * 2];
		indexArray = new GLubyte[numFaces * 3];

		for (int i=0;i<numVertices*4;i++)
		{
			colorArray[i] = 1.0f;
			colorArray_black[i] = 1.0f;
		}

		hasTexture = false;
	}

	void AddVertex(cv::Point3f vertex, cv::Scalar vertexColor, int position);

	void AddFace(GLubyte face[3], int position)
	{
		if (numFaces <= position)
		{
			LOGW(LOGTAG_OPENGL,"GLObject::AddFace - Position is greater than face numVertices");
			return;
		}

		indexArray[(position*3) + 0] = face[0];
		indexArray[(position*3) + 1] = face[1];
		indexArray[(position*3) + 2] = face[2];
	}

	~WavefrontGLObject()
	{
		delete[] colorArray;
		delete[] colorArray_black;
		delete[] vertexArray;
		delete[] textureArray;
		delete[] indexArray;
	}

	void Draw(OpenGLRenderData renderData);
	
	static WavefrontGLObject * FromObjFile(objLoader & loader);
};
#endif
