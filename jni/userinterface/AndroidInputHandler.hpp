#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "DebugSettings.hpp"
#include "android_native_app_glue.h"
#include "userinterface/events/EventDelegates.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/uimodel/UIElement.hpp"

#ifndef UIHANDLER_HPP_
#define UIHANDLER_HPP_

#define SRUTIL_DELEGATE_PREFERRED_SYNTAX

class AndroidInputHandler
{
public:
	int32_t HandleInputEvent(struct android_app* app, AInputEvent* inputEvent);
	void AddGlobalButtonDelegate(ButtonEventDelegate newDelegate);
	void AddGlobalTouchDelegate(TouchEventDelegate newDelegate);
	void SetRootUIElement(UIElement * element);
	
private:
	bool CheckEventTime(AInputEvent * inputEvent, int minEventTimeMillis);
	void HandleButtonEvent(AInputEvent * inputEvent);
	//void HandleGlobalTouchEvents(AInputEvent * inputEvent);
	bool CreateTouchEvent(AInputEvent * inputEvent, TouchEventArgs * touchEvent);

	std::vector<ButtonEventDelegate> globalButtonEventDelegates;
	std::vector<TouchEventDelegate> globalTouchEventDelegates;
	bool rootDefined;
	UIElement * rootElement;

};


#endif