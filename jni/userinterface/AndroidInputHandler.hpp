#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "AmplifyRealityGlobals.hpp"
#include "android_native_app_glue.h"
#include "userinterface/uimodel/ITextListener.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/uimodel/UIElement.hpp"

#ifndef UIHANDLER_HPP_
#define UIHANDLER_HPP_

#define SRUTIL_DELEGATE_PREFERRED_SYNTAX

class AndroidInputHandler
{
public:
	AndroidInputHandler();

	int32_t HandleInputEvent(struct android_app* app, AInputEvent* inputEvent);
	void AddGlobalButtonDelegate(ButtonEventDelegate newDelegate);
	void AddGlobalTouchDelegate(TouchEventDelegate newDelegate);
	void AddTextListener(ITextListener * newListener);
	void RemoveDelegate(TouchEventDelegate removeDelegate);
	void RemoveDelegate(ButtonEventDelegate removeDelegate);
	void SetRootUIElement(UIElement * element);
	
private:
	bool CheckEventTime(AInputEvent * inputEvent, int minEventTimeMillis);
	void HandleButtonEvent(AInputEvent * inputEvent);
	bool CreateTouchEvent(AInputEvent * inputEvent, TouchEventArgs * touchEvent);

	std::vector<ButtonEventDelegate> globalButtonEventDelegates;
	std::vector<TouchEventDelegate> globalTouchEventDelegates;
	std::vector<ITextListener*> textListeners;
	char GetCharFromKeyCode(int32_t keyCode, bool & validKey, bool upperCase);
	bool rootDefined;
	UIElement * rootElement;

	bool keyboardIsOpen;
};


#endif