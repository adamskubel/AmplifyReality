#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "DebugSettings.hpp"
#include <opencv2/core/core.hpp>
#include "userinterface/events/EventArgs.hpp"


#ifndef UIELEMENT_HPP_
#define UIELEMENT_HPP_


class UIElement
{
public:
	virtual ~UIElement()
	{
		;
	}
	virtual UIElement * GetChildAt(cv::Point2i p)
	{
		LOGD(LOGTAG_MAIN,"NullElement");
		return NULL;
	};

	virtual void HandleInput(EventArgs eventArgs) {};
	virtual void HandleInput(TouchEventArgs eventArgs) {};
	std::string Name;

};

#endif