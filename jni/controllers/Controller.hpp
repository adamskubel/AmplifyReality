#include "opencv2/highgui/highgui.hpp"

#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "DebugSettings.hpp"

#include "model/FrameItem.hpp"
#include "model/Engine.hpp"


#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

class Controller
{
public:
	virtual void ProcessFrame(Engine * engine, FrameItem * frame);
	virtual void Initialize(Engine * engine);
	virtual bool isExpired();
	virtual bool wasSuccessful();
	virtual void Render(OpenGL * openGL)
	{
		;//Do nothing
	}

protected:
	bool initialized;


};
#endif

