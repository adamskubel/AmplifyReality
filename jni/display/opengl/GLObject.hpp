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
	GLfloat *textureArray;
	GLubyte *indexArray;
	
	bool hasTexture;

	int numFaces;

public:

	WavefrontGLObject(int _numVertices, int _numFaces)
	{			
		numVertices = _numVertices;
		numFaces = _numFaces;

		vertexArray = new GLfloat[numVertices * 3];
		colorArray = new GLfloat[numVertices * 4];		
		textureArray = new GLfloat[numVertices * 2];
		indexArray = new GLubyte[numFaces * 3];

		for (int i=0;i<numVertices*4;i++)
		{
			colorArray[i] = 1.0f;
		}

		hasTexture = false;
	}

	void AddVertex(cv::Point3f vertex, cv::Scalar vertexColor, int position)
	{
		if (numVertices <= position)
		{
			LOGW(LOGTAG_OPENGL,"GLObject::AddVertex - Position is greater than vertex numVertices");
			return;
		}

		vertexArray[(position*3) + 0] = vertex.x;
		vertexArray[(position*3) + 1] = vertex.y;
		vertexArray[(position*3) + 2] = vertex.z;

		colorArray[(position*4)+0] = (GLfloat) vertexColor[0]/ 255.0f;
		colorArray[(position*4)+1] = (GLfloat) vertexColor[1]/ 255.0f;
		colorArray[(position*4)+2] = (GLfloat) vertexColor[2]/ 255.0f;
		colorArray[(position*4)+3] = (GLfloat) vertexColor[3]/ 255.0f;			
	}

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
		delete[] vertexArray;
		delete[] textureArray;
		delete[] indexArray;
	}

	void Draw(OpenGLRenderData renderData)
	{		
		if (hasTexture)
		{
			glUniform1i(renderData.useTextureFlagLocation,1);
			glEnableVertexAttribArray(renderData.textureArrayLocation);
			glVertexAttribPointer( renderData.textureArrayLocation, 2 , GL_FLOAT, 0, 0, textureArray);
		}
		else
		{
			glUniform1i(renderData.useTextureFlagLocation,0);
		}


		glEnableVertexAttribArray(renderData.vertexArrayLocation);
		glEnableVertexAttribArray(renderData.colorArrayLocation);

		glVertexAttribPointer( renderData.vertexArrayLocation,3, GL_FLOAT, 0, 0, vertexArray );		
		glVertexAttribPointer( renderData.colorArrayLocation, 4, GL_FLOAT, 0, 0, colorArray);
		
		glDrawElements(GL_TRIANGLE_STRIP,numFaces*3,GL_UNSIGNED_BYTE,indexArray);
	}


	
	static WavefrontGLObject * FromObjFile(objLoader & loader)
	{
		WavefrontGLObject * object = new WavefrontGLObject(loader.vertexCount, loader.faceCount);

		for (int i=0;i<loader.vertexCount;i++)
		{
			double * vertex = (loader.vertexList[i])->e;
			object->AddVertex(cv::Point3f((float)vertex[0],(float)vertex[1],(float)vertex[2]),Colors::RandomColor(),i);
			LOGV(LOGTAG_OPENGL,"Added vertex [%lf,%lf,%lf]",vertex[0],vertex[1],vertex[2]);
		}

		for (int i=0;i<loader.faceCount;i++)
		{
			int * vertexIndices = (loader.faceList[i])->vertex_index;
			GLubyte triangle[3] = {vertexIndices[0],vertexIndices[1],vertexIndices[2]};

			object->AddFace(triangle,i);
			LOGV(LOGTAG_OPENGL,"Added triangle [%u,%u,%u]",triangle[0],triangle[1],triangle[2]);
		}

		return object;
	}
};
#endif
