#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "AmplifyRealityGlobals.hpp"
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
		Press, Swipe, FingerDown
	};

}

class EventArgs
{
public:
	EventArgs();
};

class PhysicalButtonEventArgs : public EventArgs
{
public:
	PhysicalButtonEventArgs();
	int32_t ButtonCode;
};


class TouchEventArgs : public EventArgs
{
public:
	TouchEventArgs();
	ARInput::TouchInputType InputType;
	cv::Point2i * TouchLocations;
};

#endif