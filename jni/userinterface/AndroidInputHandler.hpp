#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "DebugSettings.hpp"
#include "android_native_app_glue.h"
#include "userinterface/uimodel/EventArgs.hpp"
#include "model/FrameItem.hpp"

#ifndef UIHANDLER_HPP_
#define UIHANDLER_HPP_

//typedef void (*UIEventCallback)(void*,EventArgs);
typedef void (*PhsyicalButtonCallback)(void*,PhysicalButtonEventArgs);
typedef void (*TouchCallback)(void*,TouchEventArgs);


class AndroidInputHandler
{
public:
	int32_t HandleInputEvent(struct android_app* app, AInputEvent* inputEvent);
	//void AddUIWatcher(UIWatcher * watcher, ARInput::InputFilter filter);

	void AddGlobalButtonListener(PhsyicalButtonCallback callback);
	void AddGlobalTouchListener(TouchCallback callback);
	
private:
	bool CheckEventTime(AInputEvent * inputEvent, int minEventTimeMillis);
	void HandleButtonEvent(AInputEvent * inputEvent);
	void HandleGlobalTouchEvents(AInputEvent * inputEvent);
	bool CreateTouchEvent(AInputEvent * inputEvent, TouchEventArgs * touchEvent);
	
	std::vector<PhsyicalButtonCallback> physicalButtonCallbacks;
	std::vector<TouchCallback> globalTouchCallbacks;


};


#endif