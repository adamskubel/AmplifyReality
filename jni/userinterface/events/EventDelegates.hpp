#include "userinterface/events/EventArgs.hpp"
#include "srutil/delegate/delegate.hpp"

#ifndef EVENTDELEGATES_HPP_
#define EVENTDELEGATES_HPP_

typedef srutil::delegate<void (void*,TouchEventArgs)> TouchEventDelegate;
typedef srutil::delegate<void (void*,PhysicalButtonEventArgs)> ButtonEventDelegate;
typedef srutil::delegate<void (void*,EventArgs)> ClickEventDelegate;


#endif