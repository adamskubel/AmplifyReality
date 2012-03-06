#include "userinterface/AndroidInputHandler.hpp"

AndroidInputHandler::AndroidInputHandler()
{
	keyboardIsOpen = false;
	rootDefined = false;
}

int32_t AndroidInputHandler::HandleInputEvent(struct android_app* app, AInputEvent * inputEvent)
{
	int32_t eventType = AInputEvent_getType(inputEvent);
	LOGV(LOGTAG_INPUT,"Handling event");


	//if (keyboardIsOpen && keyboardConfig == ACONFIGURATION_KEYSHIDDEN_YES)
	//{
	//	keyboardIsOpen = false;
	//	for (int i=0;i<textListeners.size();i++)
	//	{
	//		textListeners.at(i)->VirtualKeyboardEvent(keyboardIsOpen); 
	//	}
	//}
	//else if (!keyboardIsOpen && keyboardConfig != ACONFIGURATION_KEYSHIDDEN_NO)
	//{
	//	keyboardIsOpen = true;
	//	for (int i=0;i<textListeners.size();i++)
	//	{
	//		textListeners.at(i)->VirtualKeyboardEvent(keyboardIsOpen); 
	//	}
	//}

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
				childElement = rootElement->GetElementAt(touchEvent.TouchLocations);
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
		HandleButtonEvent(inputEvent);
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
		touchEvent->TouchLocations = cv::Point2i((int)AMotionEvent_getX(inputEvent,0),(int)AMotionEvent_getY(inputEvent,0));
		return true;
	}
	else if (eventAction == AMOTION_EVENT_ACTION_DOWN)
	{
		touchEvent->InputType = ARInput::FingerDown;
		touchEvent->TouchLocations = cv::Point2i((int)AMotionEvent_getX(inputEvent,0),(int)AMotionEvent_getY(inputEvent,0));
		return true;
	}
	return false;
}


char AndroidInputHandler::GetCharFromKeyCode(int32_t keyCode, bool & validKey, bool upperCase)
{
	if (keyCode >= AKEYCODE_A && keyCode <= AKEYCODE_Z)
	{
		int adjustedValue = ((upperCase) ? 36 : 68) + (int)keyCode;		
		validKey = true;
		return cv::saturate_cast<char>(adjustedValue);
	}
	else if (keyCode >= AKEYCODE_0 && keyCode <= AKEYCODE_9)
	{
		int adjustedValue = 41 + (int)keyCode;
		validKey = true;
		return cv::saturate_cast<char>(adjustedValue);
	}
	else
	{
		validKey = true;
		switch(keyCode)
		{
		case AKEYCODE_COMMA:
			return ',';
		case AKEYCODE_SPACE:
			return ' ';
		case AKEYCODE_SEMICOLON:
			return (upperCase) ? ':' : ';';
		case AKEYCODE_PERIOD:
			return '.';
		case AKEYCODE_SLASH:
			return '/';
		case AKEYCODE_BACKSLASH:
			return '\\';		
		default:
			LOGV(LOGTAG_INPUT,"Unsupported keycode:%d",keyCode);
			validKey = false;
			return ' ';
		}
	}
}

void AndroidInputHandler::SoftKeyboardChanged(bool _keyboardIsOpen)
{
	keyboardIsOpen = _keyboardIsOpen;
	LOGV(LOGTAG_INPUT,"Setting virtual KB state to %d",keyboardIsOpen);
	for (int i=0;i<textListeners.size();i++)
	{
		textListeners.at(i)->VirtualKeyboardEvent(keyboardIsOpen); 
	}
}

void AndroidInputHandler::HandleButtonEvent(AInputEvent * inputEvent)
{
	int32_t eventAction = AKeyEvent_getAction(inputEvent);
	int32_t eventKey = AKeyEvent_getKeyCode(inputEvent);
	int32_t flags = AKeyEvent_getFlags(inputEvent);

	//LOGD(LOGTAG_INPUT,"Key=%d",eventKey);

	//Physical buttons of interest
	if (eventKey == AKEYCODE_MENU || eventKey == AKEYCODE_SEARCH || eventKey == AKEYCODE_BACK)
	{
		

		//Check for keyboard opening/closing events
		bool lastKBState = keyboardIsOpen;
		if (eventKey == AKEYCODE_MENU && flags & AKEY_EVENT_FLAG_LONG_PRESS) //Long menu press toggles soft KB
		{
			keyboardIsOpen = !keyboardIsOpen;
		}
		if (flags & AKEY_EVENT_FLAG_SOFT_KEYBOARD) //If it came from the soft KB, then soft KB better be open!
		{
			LOGD(LOGTAG_INPUT,"Soft KB event!");
			keyboardIsOpen = true;
		}

		//If soft keyboard state has changed, notify listeners
		if (lastKBState != keyboardIsOpen)
		{			
			for (int i=0;i<textListeners.size();i++)
			{
				textListeners.at(i)->VirtualKeyboardEvent(keyboardIsOpen); 
			}
		}

		//Only care about complete press, doesn't check for down occuring
		if (eventAction == AKEY_EVENT_ACTION_UP && CheckEventTime(inputEvent,ARInput::MinimumKeyPressTime))
		{		
			for (int i=0;i<globalButtonEventDelegates.size();i++)
			{
				PhysicalButtonEventArgs buttonEvent = PhysicalButtonEventArgs();
				buttonEvent.ButtonCode = eventKey;
				globalButtonEventDelegates.at(i)(NULL,buttonEvent);
			}
		}
	}
	else
	{
		//Only care about complete press, doesn't check for down occuring
		if (eventAction == AKEY_EVENT_ACTION_UP && CheckEventTime(inputEvent,ARInput::MinimumKeyPressTime))
		{		
			int32_t metaState = AKeyEvent_getMetaState(inputEvent);
			
			bool handled = false;
			for (int i=0;i<textListeners.size();i++)
			{
				//If input is handled, unset focus of remaining handlers
				if (handled)
				{
					textListeners.at(i)->SetFocus(false);
				}
				//If not yet handled, process event
				else
				{
					bool isUpper = AMETA_SHIFT_ON & metaState;

					bool validChar = false;
					KeyEventArgs keyEvent(eventKey);
					keyEvent.KeyCharacter = GetCharFromKeyCode(eventKey,validChar,isUpper);
					keyEvent.hasCharacter = validChar;
					handled = textListeners.at(i)->HandleKeyEvent(keyEvent);
				}
			}
		}
	}
}

void AndroidInputHandler::AddTextListener(ITextListener * newListener)
{
	for (int i =0; i < textListeners.size();i++)
	{
		textListeners[i]->AddFocusChangedListener(newListener);
		newListener->AddFocusChangedListener(textListeners[i]);
	}
	textListeners.push_back(newListener);
}

void AndroidInputHandler::RemoveTextListener(ITextListener * removedListener)
{
	int eraseIndex = -1;
	for (int i =0; i < textListeners.size();i++)
	{
		if (textListeners[i] == removedListener)
			eraseIndex = i;
		else
			textListeners[i]->RemoveFocusChangedListener(removedListener);
	}
	if (eraseIndex > -1)
		textListeners.erase(textListeners.begin() + eraseIndex);
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