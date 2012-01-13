#include "model/Updateable.hpp"
#include "display/opengl/OpenGLRenderable.hpp"
#include "display/opengl/GLObject.hpp"

void Updateable::Update(FrameItem * item)
{
	LOGE("'Update' called on virtual object");
	throw new exception();
}

void OpenGLRenderable::Render(OpenGL * openGL)
{
	LOGE("'Render' called on virtual object");
	throw new exception();
}

GLObject::~GLObject()
{
	LOGE("'~GLObject' called on virtual object");
	throw new exception();
}

void GLObject::Draw()
{
	LOGE("'Draw' called on virtual object");
	throw new exception();
}
