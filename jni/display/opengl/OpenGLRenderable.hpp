#include "OpenGL.hpp"


#ifndef OPENGLRENDERABLE_HPP_
#define OPENGLRENDERABLE_HPP_

class OpenGLRenderable
{
public:
	virtual void Render(OpenGL * openGL);
	virtual ~OpenGLRenderable()
	{
		;
	}
};

#endif