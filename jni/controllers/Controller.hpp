#include "opencv2/highgui/highgui.hpp"

#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "AmplifyRealityGlobals.hpp"

#include "model/FrameItem.hpp"
#include "model/Engine.hpp"


#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

class Controller : public IDeletable
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
		return false;
	}
	virtual void SetExpired()
	{
		;
	}
	virtual void Render(OpenGL * openGL)
	{
		;
	}



};
#endif

