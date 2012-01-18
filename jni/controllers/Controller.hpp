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
	virtual ~Controller()
	{
		LOGE("Controller:: Base destructor called");
	}

	virtual void ProcessFrame(Engine * engine)
	{
		;
	}
	virtual void Initialize(Engine * engine)
	{
		;
	}
	virtual void Teardown(Engine * engine)
	{
		;
	}
	virtual bool IsExpired()
	{
		return isExpired;
	}
	virtual bool SetExpired()
	{
		isExpired = true;
	}
	virtual void Render(OpenGL * openGL)
	{
		;
	}

protected:
	bool isInitialized, isExpired;


};
#endif

