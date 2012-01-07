#include "model/Updateable.hpp"
#include "display/opengl/OpenGLRenderable.hpp"

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