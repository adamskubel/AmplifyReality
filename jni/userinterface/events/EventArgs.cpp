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