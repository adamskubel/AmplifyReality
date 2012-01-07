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


void OpenGLHelper::freeGLObject(GLObject *object) 
{
	if (object == NULL)
		return;
	delete[] object->colorArray;
	delete[] object->vertexArray;
	delete object;
}

GLObject * OpenGLHelper::CreateTexturedQuad(int textureWidth, int textureHeight, int size) 
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
	GLObject *result;
	result = newGLObject(4, 3, 2);

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

GLObject * OpenGLHelper::newGLObject(long vertices, int vertexComponents, int textureComponents) 
{
	GLObject * result = new GLObject;

	result->count = vertices;
	result->vertexComponents = vertexComponents;
	result->textureComponents = textureComponents;

	result->vertexArray = new GLfixed[vertices * vertexComponents];
	result->textureArray = new GLfixed[vertices * textureComponents];
	result->colorArray = new GLubyte[vertices * 4];

	return result;
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
