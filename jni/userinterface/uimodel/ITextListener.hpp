#ifndef ITEXT_LISTENER_HPP_
#define ITEXT_LISTENER_HPP_

#include "userinterface/events/EventArgs.hpp"
#include "srutil/delegate/delegate.hpp"

//typedef srutil::delegate<void ()> GainedFocusDelegate;

class ITextListener
{
public:
	virtual bool HandleKeyEvent(KeyEventArgs keyEvent);
	virtual void SetFocus(bool focused);
	virtual void LostFocus();
	virtual void VirtualKeyboardEvent(bool opened);
	virtual void AddFocusChangedListener(ITextListener * focusListener);
	virtual void RemoveFocusChangedListener(ITextListener * focusListener);
};
#endif