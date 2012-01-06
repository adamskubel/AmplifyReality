#include "userinterface/AndroidInputHandler.hpp"

int32_t AndroidInputHandler::HandleInputEvent(struct android_app* app, AInputEvent * inputEvent)
{
	int32_t eventType = AInputEvent_getType(inputEvent);

	switch(eventType)
	{
	//Assume motion type events are from a touch screen
	case(AINPUT_EVENT_TYPE_MOTION):
		//TODO: Send events to children
		//Handle global events
		HandleGlobalTouchEvents(inputEvent);
		break;

	case(AINPUT_EVENT_TYPE_KEY):
		//Check for physical key events, which are not contexual
		HandleButtonEvent(inputEvent);
		//Nothing else yet
		break;

	}

	return 0;
}

bool AndroidInputHandler::CreateTouchEvent(AInputEvent * inputEvent, TouchEventArgs * touchEvent)
{
	int32_t eventAction = AMotionEvent_getAction(inputEvent);

	if (eventAction == AMOTION_EVENT_ACTION_UP  && CheckEventTime(inputEvent,ARInput::MinimumTouchPressTime))
	{
		touchEvent->InputType = ARInput::Press;
		return true;
	}
	return false;
}

void AndroidInputHandler::HandleGlobalTouchEvents(AInputEvent * inputEvent)
{
	TouchEventArgs touchEvent = TouchEventArgs();

	if (CreateTouchEvent(inputEvent, &touchEvent))
	{
		for (int i=0;i<globalTouchCallbacks.size();i++)
		{
			LOGD(LOGTAG_INPUT,"Sending touch event to callback: %d",i);
			globalTouchCallbacks.at(i)(NULL,touchEvent);
		}
	}
}

void AndroidInputHandler::HandleButtonEvent(AInputEvent * inputEvent)
{
	int32_t eventAction = AKeyEvent_getAction(inputEvent);
	int32_t eventKey = AKeyEvent_getKeyCode(inputEvent);
	//Ignore home, other three physical buttons are hard-defined
	if (eventKey == AKEYCODE_HOME || eventKey == AKEYCODE_MENU || eventKey == AKEYCODE_SEARCH)
	{
		//Only care about complete press, doesn't check for down occuring
		if (eventAction == AKEY_EVENT_ACTION_UP && CheckEventTime(inputEvent,ARInput::MinimumKeyPressTime))
		{		
			for (int i=0;i<physicalButtonCallbacks.size();i++)
			{
				PhysicalButtonEventArgs buttonEvent = PhysicalButtonEventArgs();
				buttonEvent.ButtonCode = eventKey;
				LOGD(LOGTAG_INPUT,"Sending button event to callback: %d",i);
				physicalButtonCallbacks.at(i)(NULL,buttonEvent);
			}
		}
	}
}

void AndroidInputHandler::AddGlobalButtonListener(PhsyicalButtonCallback callback)
{
	physicalButtonCallbacks.push_back(callback);
}

void AndroidInputHandler::AddGlobalTouchListener(TouchCallback callback)
{
	globalTouchCallbacks.push_back(callback);
}

bool AndroidInputHandler::CheckEventTime(AInputEvent * inputEvent, int eventTimeMillis)
{
	if (AInputEvent_getType(inputEvent) == AINPUT_EVENT_TYPE_MOTION)
	{
		return ((float) (AMotionEvent_getEventTime(inputEvent) / 1000000LL)) > eventTimeMillis;
	
	} else if (AInputEvent_getType(inputEvent) == AINPUT_EVENT_TYPE_KEY)
	{
		return ((float) (AKeyEvent_getEventTime(inputEvent) / 1000000LL)) > eventTimeMillis;
	}
}