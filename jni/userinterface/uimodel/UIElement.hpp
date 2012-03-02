#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "AmplifyRealityGlobals.hpp"
#include <opencv2/core/core.hpp>
#include "userinterface/events/EventArgs.hpp"
#include "model/IDeletable.hpp"
#include "display/Colors.hpp"


#ifndef UIELEMENT_HPP_
#define UIELEMENT_HPP_

namespace HorizontalAlignments
{
	enum HorizontalAlignment
	{
		Left, Center, Right
	};
}

namespace VerticalAlignments
{
	enum VerticalAlignment
	{
		Top, Center, Bottom
	};
}

class UIElement : public IDeletable
{
public:
	virtual ~UIElement()
	{
		;
	}
	virtual UIElement * GetElementAt(cv::Point2i p)
	{
		LOGD(LOGTAG_MAIN,"NullElement");
		return NULL;
	};

	virtual void HandleInput(EventArgs eventArgs) {};
	virtual void HandleInput(TouchEventArgs eventArgs) {};
	std::string Name;

	void * Tag;
	

};

#endif