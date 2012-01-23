#include "display/opengl/OpenGLHelper.hpp"

/* Following gluLookAt implementation is adapted from the
* Mesa 3D Graphics library. http://www.mesa3d.org
*/
void OpenGLHelper::gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
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

void OpenGLHelper::PopulateVertices(GLObject * glObject, vector<cv::Point3f> * vertices)
{
	for (int i=0;i<vertices->size(); i++)
	{
		glObject->vertexArray[(i*3)+0] = floatToFixed(vertices->at(i).x); //x
		glObject->vertexArray[(i*3)+1] = floatToFixed(vertices->at(i).y); //y
		glObject->vertexArray[(i*3)+2] = floatToFixed(vertices->at(i).z); //z
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
			
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+0] = (GLubyte) colors->at(colorVectorIndex)[0];
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+1] = (GLubyte) colors->at(colorVectorIndex)[1];
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+2] = (GLubyte) colors->at(colorVectorIndex)[2];
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+3] = (GLubyte) colors->at(colorVectorIndex)[3];			
		}
	}
	//There are enough colors for each face
	else
	{
		LOGW(LOGTAG_OPENGL,"Enough colors for each vertex");
		for (int i=0;i<colorObject->count;i++)
		{
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+0] = (GLubyte) colors->at(i)[0];
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+1] = (GLubyte) colors->at(i)[1];
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+2] = (GLubyte) colors->at(i)[2];
			colorObject->colorArray[(ColorGLObject::colorComponents*i)+3] = (GLubyte) colors->at(i)[3];
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

// Capped conversion from float to fixed.
long OpenGLHelper::floatToFixed(float value) 
{
	if (value < -32768)
		value = -32768;
	if (value > 32767)
		value = 32767;
	return (long) (value * 65536);
}

//Creates a perspective camera matrix centered at the origin facing (0,0,1)
//FOV is in degrees
void OpenGLHelper::gluPerspective(GLfloat fovy,GLfloat aspectRatio,  GLfloat zNear, GLfloat zFar)
{
	GLfloat fovy_RAD = (PI/180.0f) * fovy;

	GLfloat top = -zNear * tanf(fovy_RAD/2.0f);  
	GLfloat left = top * aspectRatio;
	//(left,top) is the upper left corner
	GLfloat right = -left;
	GLfloat bottom = -top;

	glFrustumf(left,right,bottom,top,zNear,zFar);
	LOGV(LOGTAG_OPENGL,"Created frustum: fovy=%f,fovy(RADIANS)=%f,left=%f,top=%f",fovy,fovy_RAD,left,top);
}