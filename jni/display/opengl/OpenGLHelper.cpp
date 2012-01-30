#include "display/opengl/OpenGLHelper.hpp"


TexturedGLObject * OpenGLHelper::CreateTexturedQuad(int textureWidth, int textureHeight, int size) 
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
	TexturedGLObject * result = new TexturedGLObject(4, 2);

	result->textureArray[0] = 0;
	result->textureArray[1] = 1.0f;

	result->textureArray[2] = 0;
	result->textureArray[3] = 0;

	result->textureArray[4] = 1.0f;
	result->textureArray[5] = 1.0f;

	result->textureArray[6] = 1.0f;
	result->textureArray[7] = 0;

	//result->textureArray[0] = 0;
	//result->textureArray[1] = 0;

	//result->textureArray[2] = 0;
	//result->textureArray[3] = 1.0f;

	//result->textureArray[4] = 1.0f;
	//result->textureArray[5] = 0;

	//result->textureArray[6] = 1.0f;
	//result->textureArray[7] = 1.0f;




	result->vertexArray[0] = 0; //x
	result->vertexArray[1] = 0; //y
	result->vertexArray[2] = 0; //z

	result->vertexArray[3] = 0;
	result->vertexArray[4] = yLimit;
	result->vertexArray[5] = 0;

	result->vertexArray[6] = xLimit;
	result->vertexArray[7] = 0;
	result->vertexArray[8] = 0;

	result->vertexArray[9] = xLimit;
	result->vertexArray[10] = yLimit;
	result->vertexArray[11] = 0;

	result->width = xLimit;
	result->height = yLimit;

	LOGD("OpenGL","Image quad complete");
	return result;
}

void OpenGLHelper::PopulateVertices(GLObject * glObject, vector<cv::Point3f> * vertices)
{
	for (int i=0;i<vertices->size(); i++)
	{
		glObject->vertexArray[(i*3)+0] = vertices->at(i).x; //x
		glObject->vertexArray[(i*3)+1] = vertices->at(i).y; //y
		glObject->vertexArray[(i*3)+2] = vertices->at(i).z; //z
	}
}

void OpenGLHelper::PopulateColors(ColorGLObject * colorObject, vector<cv::Scalar> * colors)
{
	if (colors == NULL || colors->empty())
	{
		LOGE("Null or empty vector passed to PopulateColors!");
		return;
	}
	//More vertices than colors, so populate until color vector is exhausted, then continue
	//populating using the last value in the color vector
	else if (colors->size() < colorObject->count)
	{
		LOGW(LOGTAG_OPENGL,"More vertices than colors");
		int colorVectorIndex = 0;
		for (int i=0;i<colorObject->count ;i++)
		{
			colorVectorIndex = (i < colors->size()) ? i : colors->size() -1;
			
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+0] = (GLfloat) colors->at(colorVectorIndex)[0]/ 255.0f;
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+1] = (GLfloat) colors->at(colorVectorIndex)[1]/ 255.0f;
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+2] = (GLfloat) colors->at(colorVectorIndex)[2]/ 255.0f;
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+3] = (GLfloat) colors->at(colorVectorIndex)[3]/ 255.0f;			
		}
	}
	//There are enough colors for each face
	else
	{
		LOGW(LOGTAG_OPENGL,"Enough colors for each vertex");
		for (int i=0;i<colorObject->count;i++)
		{
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+0] = (GLfloat) colors->at(i)[0]/ 255.0f;
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+1] = (GLfloat) colors->at(i)[1]/ 255.0f;
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+2] = (GLfloat) colors->at(i)[2]/ 255.0f;
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+3] = (GLfloat) colors->at(i)[3]/ 255.0f;
		}
	}
}

ColorGLObject * OpenGLHelper::CreateMultiColorCube(int _size)
{
	ColorGLObject* cube = CreateCube(_size);

	vector<cv::Scalar> colorVector;
	colorVector.push_back(Colors::MidnightBlue);
	colorVector.push_back(Colors::MidnightBlue);
	colorVector.push_back(Colors::MidnightBlue);
	colorVector.push_back(Colors::MidnightBlue); 
		
	colorVector.push_back(Colors::CornflowerBlue);
	colorVector.push_back(Colors::CornflowerBlue);
	colorVector.push_back(Colors::CornflowerBlue);
	colorVector.push_back(Colors::CornflowerBlue);
		
	colorVector.push_back(Colors::OliveDrab);
	colorVector.push_back(Colors::OliveDrab);
	colorVector.push_back(Colors::OliveDrab);
	colorVector.push_back(Colors::OliveDrab);
	
	colorVector.push_back(Colors::Green);
	colorVector.push_back(Colors::Green);
	colorVector.push_back(Colors::Green);
	colorVector.push_back(Colors::Green);

	colorVector.push_back(Colors::Gold);
	colorVector.push_back(Colors::Gold);
	colorVector.push_back(Colors::Gold);
	colorVector.push_back(Colors::Gold);
	
	colorVector.push_back(Colors::Aqua);
	colorVector.push_back(Colors::Aqua);
	colorVector.push_back(Colors::Aqua);
	colorVector.push_back(Colors::Aqua);

	PopulateColors(cube,&colorVector);

	return cube;
}

ColorGLObject * OpenGLHelper::CreateSolidColorCube(int _size, cv::Scalar color)
{
	ColorGLObject* cube = CreateCube(_size);
	vector<cv::Scalar> colorVector;
	colorVector.push_back(color);
	PopulateColors(cube,&colorVector);
	return cube;
}

ColorGLObject * OpenGLHelper::CreateCube(int _size)
{
	LOGD(LOGTAG_OPENGL,"Creating cube");
	ColorGLObject *glObject = new ColorGLObject(24,4);

	float size = (float)_size/2.0f;

	vector<cv::Point3f> vertices = vector<cv::Point3f>();
		
	vertices.push_back(cv::Point3f(-size, -size,  size));
	vertices.push_back(cv::Point3f(size, -size,  size));
	vertices.push_back(cv::Point3f(-size,  size,  size));
	vertices.push_back(cv::Point3f(size,  size,  size));

	vertices.push_back(cv::Point3f(-size, -size, -size));
	vertices.push_back(cv::Point3f(-size,  size, -size));
	vertices.push_back(cv::Point3f(size, -size, -size));
	vertices.push_back(cv::Point3f(size,  size, -size));	

	vertices.push_back(cv::Point3f(-size, -size,  size));
	vertices.push_back(cv::Point3f(-size,  size,  size));
	vertices.push_back(cv::Point3f(-size, -size, -size));
	vertices.push_back(cv::Point3f(-size,  size, -size));

	vertices.push_back(cv::Point3f(size, -size, -size));
	vertices.push_back(cv::Point3f(size,  size, -size));
	vertices.push_back(cv::Point3f(size, -size,  size));
	vertices.push_back(cv::Point3f(size,  size,  size));

	vertices.push_back(cv::Point3f(-size,  size,  size));
	vertices.push_back(cv::Point3f(size,  size,  size));
	vertices.push_back(cv::Point3f(-size,  size, -size));
	vertices.push_back(cv::Point3f(size,  size, -size));

	vertices.push_back(cv::Point3f(-size, -size,  size));
	vertices.push_back(cv::Point3f(-size, -size, -size));
	vertices.push_back(cv::Point3f(size, -size,  size));
	vertices.push_back(cv::Point3f(size, -size, -size));
		
	LOGD(LOGTAG_OPENGL,"Vertice vector is %d long",vertices.size());
	PopulateVertices(glObject,&vertices);

	

	glObject->width = 2*size;
	glObject->height = 2*size;

	LOGD(LOGTAG_OPENGL,"Blank cube complete");
	return glObject;
}


//Creates a perspective camera matrix centered at the origin facing (0,0,1)
//FOV is in degrees
void OpenGLHelper::gluPerspective(Mat & matrix, GLfloat fovy,GLfloat aspectRatio,  GLfloat zNear, GLfloat zFar)
{
	float fovy_RAD = (PI/180.0f) * fovy;

	float top = -zNear * tanf(fovy_RAD/2.0f);  
	float left = top * aspectRatio;
	//(left,top) is the upper left corner
	float right = -left;
	float bottom = -top;

	createFrustum(matrix, left,right,bottom,top,zNear,zFar);
	LOGV(LOGTAG_OPENGL,"Created frustum: fovy=%f,fovy(RADIANS)=%f,left=%f,top=%f",fovy,fovy_RAD,left,top);
}

void OpenGLHelper::createFrustum(Mat & matrix, float left, float right, float bottom, float top, float nearVal, float farVal)
{
	float A = (right+left)/(right-left);
	float B = (top + bottom)/(top - bottom);
	float C = -(farVal+nearVal)/(farVal-nearVal);
	float D =  -(2.0f * farVal * nearVal) / (farVal - nearVal);

	float data[] = 
	{
		(2.0f*nearVal)/(right-left), 0,  A, 0 ,
		0, (2.0f*nearVal)/(top-bottom), B, 0, 
		0, 0, C, D,
		0, 0, -1, 0
	};

	
	Mat frustumMmatrix = Mat(4,4,CV_32F,data);
	matrix *= frustumMmatrix;
}

void OpenGLHelper::translate(Mat & matrix, Point3f point)
{
	float data[] = 
	{
		1,0,0,point.x,
		0,1,0,point.y,
		0,0,1,point.z,
		0,0,0,1
	};

	Mat translation(4,4,CV_32F,data);
		

	matrix *= translation;
}

void OpenGLHelper::rotate(Mat & matrix, float rotation, Point3f rotationVector)
{
	float x = rotationVector.x;
	float y = rotationVector.y;
	float z = rotationVector.z;

	float c= cos(rotation);
	float c1 = 1.0f - c;
	float s= sin(rotation);

	float data[] = 
	{
		pow(x,2)*c1+c, x*y*c1 - (z*s), x*z*c1 + (y*s), 0,
		y*x*c1 + (z*s), pow(y,2)*c1+c, y*z*c1-(x*s), 0,
		x*z*c1 - (y*s), y*z*c1 + (x*s), pow(z,2)*c1 + c, 0,
		0,0,0,1
	};
		
	Mat rotationMatrix(4,4,CV_32F,data);

	matrix *= rotationMatrix;
}

void OpenGLHelper::createOrtho(Mat & matrix, float left, float right, float bottom, float top, float nearVal, float farVal)
{
	float tx = - ((right+left)/(right-left));
	float ty = - ((top+bottom)/(top-bottom));
	float tz = - ((farVal+nearVal)/(farVal-nearVal));

	float data[] = 
	{
		2.0f / (right-left),	0,							0,	tx,
		0,						(2.0f/(top-bottom)),		0,  ty,
		0,						0,	 (-2.0f/(farVal-nearVal)),	tz,
		0,						0,							0,	1
	};

	Mat orthoMatrix = Mat(4,4,CV_32F,data);
	matrix *= orthoMatrix;
}

void OpenGLHelper::scale(Mat & matrix, Point3f scale)
{
	float data[] = 
	{
		scale.x,	0,			0,			0,
		0,			scale.y,	0,			0,
		0,			0,			scale.z,	0,
		0,			0,			0,			1
	};
	Mat scaleMatrix(4,4,CV_32F,data);

	matrix *= scaleMatrix;
}