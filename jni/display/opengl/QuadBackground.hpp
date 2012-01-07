#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "DebugSettings.hpp"
#include "display/opengl/OpenGLRenderable.hpp"
#include "display/opengl/OpenGL.hpp"
#include "display/opengl/OpenGLHelper.hpp"
#include "model/Updateable.hpp"


#ifndef QUADBACKGROUND_HPP_
#define QUADBACKGROUND_HPP_

class QuadBackground : public OpenGLRenderable, public Updateable
{
public:
	QuadBackground(int imageWidth, int imageHeight);
	~QuadBackground();
	void Render(OpenGL * openGL);
	void Update(FrameItem * item);


private:
	GLuint textureID;
	GLObject * texturedQuad;
	static const int bgSize = 20; 
	int textureWidth, textureHeight, imageWidth, imageHeight;
	void * imagePixels;

	void drawTexturedObject(GLObject *object);
	void calculateTextureSize(int imageWidth, int imageHeight, int * textureWidth, int * textureHeight);
	void SetMatrices(OpenGL * openGL);
	void CreateTexture(int imageWidth, int imageHeight);
};

#endif