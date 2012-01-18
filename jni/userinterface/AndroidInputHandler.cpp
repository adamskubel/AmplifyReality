#include "userinterface/AndroidInputHandler.hpp"

int32_t AndroidInputHandler::HandleInputEvent(struct android_app* app, AInputEvent * inputEvent)
{
	int32_t eventType = AInputEvent_getType(inputEvent);

	//Assume motion type events are from a touch screen
	if (eventType == AINPUT_EVENT_TYPE_MOTION)
	{
		TouchEventArgs touchEvent = TouchEventArgs();
		if (CreateTouchEvent(inputEvent, &touchEvent))
		{
			//Send events to children
			UIElement * childElement = NULL;
			if (rootElement != NULL)
			{
				childElement = rootElement->GetChildAt(*(touchEvent.TouchLocations));
				if (childElement != NULL)
					childElement->HandleInput(touchEvent);
				else
				{
					LOGD(LOGTAG_INPUT,"No children found");
				}
			}
			//If no children found, process as a global event
			if (childElement == NULL)
			{
				for (int i=0;i<globalTouchEventDelegates.size();i++)
				{
					LOGD(LOGTAG_INPUT,"Sending touch event to callback: %d",i);
					globalTouchEventDelegates.at(i)(NULL,touchEvent);
				}
			}
		}			
	}
	else if ( eventType == AINPUT_EVENT_TYPE_KEY)
	{
		//Check for physical key events, which are not contexual
		HandleButtonEvent(inputEvent);
		//Nothing else yet
	}
	return 0;
}


void AndroidInputHandler::SetRootUIElement(UIElement * element)
{
	LOGD(LOGTAG_INPUT,"Root UI Element set");
	rootElement = element;
}

bool AndroidInputHandler::CreateTouchEvent(AInputEvent * inputEvent, TouchEventArgs * touchEvent)
{
	int32_t eventAction = AMotionEvent_getAction(inputEvent);

	if (eventAction == AMOTION_EVENT_ACTION_UP  && CheckEventTime(inputEvent,ARInput::MinimumTouchPressTime))
	{
		touchEvent->InputType = ARInput::Press;
		touchEvent->TouchLocations = new cv::Point2i((int)AMotionEvent_getX(inputEvent,0),(int)AMotionEvent_getY(inputEvent,0));
		return true;
	}
	else if (eventAction == AMOTION_EVENT_ACTION_DOWN)
	{
		touchEvent->InputType = ARInput::FingerDown;
		touchEvent->TouchLocations = new cv::Point2i((int)AMotionEvent_getX(inputEvent,0),(int)AMotionEvent_getY(inputEvent,0));
		return true;
	}
	return false;
}

//void AndroidInputHandler::HandleGlobalTouchEvents(AInputEvent * inputEvent)
//{
//	TouchEventArgs touchEvent = TouchEventArgs();
//
//	if (CreateTouchEvent(inputEvent, &touchEvent))
//	{
//		for (int i=0;i<globalTouchEventDelegates.size();i++)
//		{
//			LOGD(LOGTAG_INPUT,"Sending touch event to callback: %d",i);
//			globalTouchEventDelegates.at(i)(NULL,touchEvent);
//		}
//	}
//}

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
			for (int i=0;i<globalButtonEventDelegates.size();i++)
			{
				PhysicalButtonEventArgs buttonEvent = PhysicalButtonEventArgs();
				buttonEvent.ButtonCode = eventKey;
				LOGD(LOGTAG_INPUT,"Sending button event to callback: %d",i);
				globalButtonEventDelegates.at(i)(NULL,buttonEvent);
			}
		}
	}
}

void AndroidInputHandler::AddGlobalButtonDelegate(ButtonEventDelegate myDelegate)
{
	globalButtonEventDelegates.push_back(myDelegate);
}

void AndroidInputHandler::AddGlobalTouchDelegate(TouchEventDelegate myDelegate)
{
	globalTouchEventDelegates.push_back(myDelegate);
}

void AndroidInputHandler::RemoveDelegate(ButtonEventDelegate removeDelegate)
{
	for (int i=0;i<globalButtonEventDelegates.size();i++)
	{
		if (globalButtonEventDelegates.at(i) == removeDelegate)
		{
			globalButtonEventDelegates.erase(globalButtonEventDelegates.begin()+i);
			LOGD(LOGTAG_INPUT,"Removed button event delegate at position: %d",i);
			break;
		}
	}
}

void AndroidInputHandler::RemoveDelegate(TouchEventDelegate removeDelegate)
{	
	for (int i=0;i<globalTouchEventDelegates.size();i++)
	{
		if (globalTouchEventDelegates.at(i) == removeDelegate)
		{
			globalTouchEventDelegates.erase(globalTouchEventDelegates.begin()+i);
			LOGD(LOGTAG_INPUT,"Removed touch event delegate at position: %d",i);
			break;
		}
	}
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