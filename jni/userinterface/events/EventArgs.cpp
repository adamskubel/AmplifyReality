#include "userinterface/events/EventArgs.hpp"

//
//EventArgs::EventArgs()
//{
//
//}

TouchEventArgs::TouchEventArgs()
{
	InputType = ARInput::Press;
	TouchLocations = NULL;
}

PhysicalButtonEventArgs::PhysicalButtonEventArgs()
{
	ButtonCode = 0;
}

EventArgs::EventArgs()
{
	;
}

KeyEventArgs::KeyEventArgs(int32_t _keyCode)
{
	keyCode = _keyCode;
	hasCharacter = false;
}

KeyEventArgs::KeyEventArgs(int32_t _keyCode, char keyCharacter)
{
	keyCode = _keyCode;
	KeyCharacter = keyCharacter;
	hasCharacter = true;
}