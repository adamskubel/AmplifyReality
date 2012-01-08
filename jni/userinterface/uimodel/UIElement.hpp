#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include "DebugSettings.hpp"
#include <opencv2/core/core.hpp>


#ifndef UIELEMENT_HPP_
#define UIELEMENT_HPP_


class UIElement
{
public:
	virtual UIElement * GetChildAt(cv::Point2i p)
	{
		LOGD(LOGTAG_MAIN,"NullElement");
		return NULL;
	};
	virtual void HandleInput() {};
	std::string Name;

};

#endif