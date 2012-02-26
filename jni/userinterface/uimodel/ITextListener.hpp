#ifndef ITEXT_LISTENER_HPP_
#define ITEXT_LISTENER_HPP_

#include "userinterface/events/EventArgs.hpp"

class ITextListener
{
public:
	virtual void HandleKeyEvent(KeyEventArgs keyEvent);
	virtual void VirtualKeyboardEvent(bool opened);
};
#endif