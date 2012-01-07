#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "DebugSettings.hpp"
#include <opencv2/core/core.hpp>


#ifndef EVENTARGS_HPP_
#define EVENTARGS_HPP_


namespace ARInput
{
	static const int MinimumKeyPressTime = 500;
	static const int MinimumTouchPressTime = 800;
	enum InputFilter
	{
		All, Button, Touch
	};

	enum TouchInputType
	{
		Press, Swipe
	};

}

class EventArgs
{
public:
	EventArgs();
};

class PhysicalButtonEventArgs// : EventArgs
{
public:
	PhysicalButtonEventArgs();
	int32_t ButtonCode;
};


class TouchEventArgs// : EventArgs
{
public:
	TouchEventArgs();
	ARInput::TouchInputType InputType;
	cv::Point2i * TouchLocations;
};

#endif