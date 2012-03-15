#include "GLObject.hpp"

void WavefrontGLObject::Draw(OpenGLRenderData renderData)
{
	if (hasTexture)
	{
		glUniform1i(renderData.useTextureFlagLocation, 1);
		glEnableVertexAttribArray(renderData.textureArrayLocation);
		glVertexAttribPointer(renderData.textureArrayLocation, 2, GL_FLOAT, 0, 0, textureArray);
	} else
	{
		glUniform1i(renderData.useTextureFlagLocation, 0);
	}

	glEnableVertexAttribArray(renderData.vertexArrayLocation);
	glEnableVertexAttribArray(renderData.colorArrayLocation);

	
	/*glVertexAttribPointer(renderData.vertexArrayLocation, 3, GL_FLOAT, 0, 0, vertexArray);
	glVertexAttribPointer(renderData.colorArrayLocation, 4, GL_FLOAT, 0, 0, colorArray);
	glDrawElements(GL_TRIANGLE_STRIP, numFaces * 3, GL_UNSIGNED_BYTE, indexArray);	*/
	
	/*if (numFaces > 10)
	{*/
	glVertexAttribPointer(renderData.vertexArrayLocation, 3, GL_FLOAT, 0, 0, vertexArray);
	glVertexAttribPointer(renderData.colorArrayLocation, 4, GL_FLOAT, 0, 0, colorArray);
	glLineWidth(2);
	glDrawElements(GL_LINE_STRIP, numFaces * 3, GL_UNSIGNED_BYTE, indexArray);

}

void WavefrontGLObject::AddVertex(cv::Point3f vertex, cv::Scalar vertexColor, int position)
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
	
	colorArray_black[(position*4)+0] = (GLfloat) 0;
	colorArray_black[(position*4)+1] = (GLfloat) 0;
	colorArray_black[(position*4)+2] = (GLfloat) 0;
	colorArray_black[(position*4)+3] = (GLfloat) 0;		
}


WavefrontGLObject * WavefrontGLObject::FromObjFile(objLoader & loader)
{
	WavefrontGLObject * object = new WavefrontGLObject(loader.vertexCount, loader.faceCount);

	cv::Scalar friendlyColors[] =
	{ Colors::Red, Colors::Blue, Colors::Lime};
	cv::Scalar faceColor = Colors::DeepSkyBlue;//friendlyColors[std::rand() % 3];

	for (int i = 0; i < loader.vertexCount; i++)
	{
		double * vertex = (loader.vertexList[i])->e;
		object->AddVertex(cv::Point3f((float) vertex[0], (float) vertex[1], (float) vertex[2]), faceColor, i);
		LOGV(LOGTAG_OPENGL, "Added vertex [%lf,%lf,%lf]", vertex[0], vertex[1], vertex[2]);
	}

	for (int i = 0; i < loader.faceCount; i++)
	{
		int * vertexIndices = (loader.faceList[i])->vertex_index;
		GLubyte triangle[3] =
		{ vertexIndices[0], vertexIndices[1], vertexIndices[2] };

		object->AddFace(triangle, i);
		LOGV(LOGTAG_OPENGL, "Added triangle [%u,%u,%u]", triangle[0], triangle[1], triangle[2]);
	}

	return object;
}
