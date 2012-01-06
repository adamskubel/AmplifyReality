#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "DebugSettings.hpp"

#include "model/Engine.hpp"
#include "model/FrameItem.hpp"


#ifndef UIELEMENT_HPP_
#define UIELEMENT_HPP_

class Element
{
public:
	virtual void Render(Engine* engine, FrameItem * item);
	virtual Element GetChildAt(Point2i p);


};

#endif