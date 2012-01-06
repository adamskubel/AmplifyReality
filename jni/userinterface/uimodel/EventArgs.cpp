#include "userinterface/uimodel/EventArgs.hpp"

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